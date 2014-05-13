/*
 * sunxi-ss.c - hardware cryptographic accelerator for Allwinner A20 SoC
 *
 * Copyright (C) 2013-2014 Corentin LABBE <clabbe.montjoie@gmail.com>
 *
 * Support AES cipher with 128,192,256 bits keysize.
 * Support MD5 and SHA1 hash algorithms.
 * Support DES and 3DES
 * Support PRNG
 *
 * You could find the datasheet at
 * http://dl.linux-sunxi.org/A20/A20%20User%20Manual%202013-03-22.pdf
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License
 */

#include <linux/clk.h>
#include <linux/crypto.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <crypto/scatterwalk.h>
#include <linux/scatterlist.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_MD5
#include <crypto/md5.h>
#define SUNXI_SS_HASH_COMMON
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_SHA1
#include <crypto/sha.h>
#define SUNXI_SS_HASH_COMMON
#endif
#ifdef SUNXI_SS_HASH_COMMON
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_AES
#include <crypto/aes.h>
#endif

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_3DES
#define SUNXI_SS_DES
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_DES
#define SUNXI_SS_DES
#endif
#ifdef SUNXI_SS_DES
#include <crypto/des.h>
#endif

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_PRNG
#include <crypto/internal/rng.h>

struct prng_context {
	u8 seed[192/8];
	unsigned int slen;
};
#endif

#define SUNXI_SS_CTL            0x00
#define SUNXI_SS_KEY0           0x04
#define SUNXI_SS_KEY1           0x08
#define SUNXI_SS_KEY2           0x0C
#define SUNXI_SS_KEY3           0x10
#define SUNXI_SS_KEY4           0x14
#define SUNXI_SS_KEY5           0x18
#define SUNXI_SS_KEY6           0x1C
#define SUNXI_SS_KEY7           0x20

#define SUNXI_SS_IV0            0x24
#define SUNXI_SS_IV1            0x28
#define SUNXI_SS_IV2            0x2C
#define SUNXI_SS_IV3            0x30

#define SUNXI_SS_CNT0           0x34
#define SUNXI_SS_CNT1           0x38
#define SUNXI_SS_CNT2           0x3C
#define SUNXI_SS_CNT3           0x40

#define SUNXI_SS_FCSR           0x44
#define SUNXI_SS_ICSR           0x48

#define SUNXI_SS_MD0            0x4C
#define SUNXI_SS_MD1            0x50
#define SUNXI_SS_MD2            0x54
#define SUNXI_SS_MD3            0x58
#define SUNXI_SS_MD4            0x5C

#define SS_RXFIFO         0x200
#define SS_TXFIFO         0x204

/* SUNXI_SS_CTL configuration values */

/* AES/DES/3DES key select - bits 24-27 */
#define SUNXI_SS_KEYSELECT_KEYN		(0 << 24)

/* PRNG generator mode - bit 15 */
#define SUNXI_PRNG_ONESHOT              (0 << 15)
#define SUNXI_PRNG_CONTINUE             (1 << 15)

/* IV Steady of SHA-1/MD5 constants - bit 14 */
#define SUNXI_SS_IV_CONSTANTS              (0 << 14)
#define SUNXI_IV_ARBITRARY              (1 << 14)

/* SS operation mode - bits 12-13 */
#define SUNXI_SS_ECB                    (0 << 12)
#define SUNXI_SS_CBC                    (1 << 12)
#define SUNXI_SS_CNT                    (2 << 12)

/* Counter width for CNT mode - bits 10-11 */
#define SUNXI_CNT_16BITS                (0 << 10)
#define SUNXI_CNT_32BITS                (1 << 10)
#define SUNXI_CNT_64BITS                (2 << 10)

/* Key size for AES - bits 8-9 */
#define SUNXI_AES_128BITS               (0 << 8)
#define SUNXI_AES_192BITS               (1 << 8)
#define SUNXI_AES_256BITS               (2 << 8)

/* Operation direction - bit 7 */
#define SUNXI_SS_ENCRYPTION             (0 << 7)
#define SUNXI_SS_DECRYPTION             (1 << 7)

/* SS Method - bits 4-6 */
#define SUNXI_OP_AES                    (0 << 4)
#define SUNXI_OP_DES                    (1 << 4)
#define SUNXI_OP_3DES                   (2 << 4)
#define SUNXI_OP_SHA1                   (3 << 4)
#define SUNXI_OP_MD5                    (4 << 4)
#define SUNXI_OP_PRNG                   (5 << 4)

/* Data end bit - bit 2 */
#define SUNXI_SS_DATA_END               BIT(2)

/* PRNG start bit - bit 1 */
#define SUNXI_PRNG_START                BIT(1)

/* SS Enable bit - bit 0 */
#define SUNXI_SS_DISABLED               (0 << 0)
#define SUNXI_SS_ENABLED                (1 << 0)

/* RX FIFO status - bit 30 */
#define SS_RXFIFO_FREE               BIT(30)

/* RX FIFO empty spaces - bits 24-29 */
#define SS_RXFIFO_SPACES(val)        (((val) >> 24) & 0x3f)

/* TX FIFO status - bit 22 */
#define SS_TXFIFO_AVAILABLE          BIT(22)

/* TX FIFO available spaces - bits 16-21 */
#define SS_TXFIFO_SPACES(val)        (((val) >> 16) & 0x3f)

#define SUNXI_RXFIFO_EMP_INT_PENDING    BIT(10)
#define SUNXI_TXFIFO_AVA_INT_PENDING    BIT(8)
#define SUNXI_RXFIFO_EMP_INT_ENABLE     BIT(2)
#define SUNXI_TXFIFO_AVA_INT_ENABLE     BIT(0)

#define SUNXI_SS_ICS_DRQ_ENABLE         BIT(4)

/* General notes:
 * I cannot use a key/IV cache because each time one of these change ALL stuff
 * need to be re-writed.
 * And for example, with dm-crypt IV changes on each request.
 *
 * After each request the device must be disabled.
 *
 * For performance reason, we use writel_relaxed/read_relaxed for all
 * operations on RX and TX FIFO.
 * For all other registers, we use writel.
 * */

static struct sunxi_ss_ctx {
	void *base;
	int irq;
	struct clk *busclk;
	struct clk *ssclk;
	struct device *dev;
	struct resource *res;
	void *buf_in; /* pointer to data to be uploaded to the device */
	size_t buf_in_size; /* size of buf_in */
	void *buf_out;
	size_t buf_out_size;
} _ss_ctx, *ss_ctx = &_ss_ctx;

static DEFINE_MUTEX(lock);
static DEFINE_MUTEX(bufout_lock);
static DEFINE_MUTEX(bufin_lock);

struct sunxi_req_ctx {
	u8 key[AES_MAX_KEY_SIZE * 8];
	u32 keylen;
	u32 mode;
	u64 byte_count; /* number of bytes "uploaded" to the device */
	u32 waitbuf; /* a partial word waiting to be completed and
			uploaded to the device */
	/* number of bytes to be uploaded in the waitbuf word */
	unsigned int nbwait;
};

#ifdef SUNXI_SS_HASH_COMMON
/*============================================================================*/
/*============================================================================*/
/* sunxi_hash_init: initialize request context
 * Activate the SS, and configure it for MD5 or SHA1
 */
static int sunxi_shash_init(struct shash_desc *desc)
{
	const char *hash_type;
	struct sunxi_req_ctx *op = crypto_shash_ctx(desc->tfm);
	u32 tmp = SUNXI_SS_ENABLED | SUNXI_SS_IV_CONSTANTS;

	mutex_lock(&lock);

	hash_type = crypto_tfm_alg_name(crypto_shash_tfm(desc->tfm));

	op->byte_count = 0;
	op->nbwait = 0;
	op->waitbuf = 0;

	/* Enable and configure SS for MD5 or SHA1 */
	if (strcmp(hash_type, "sha1") == 0) {
		tmp |= SUNXI_OP_SHA1;
		op->mode = SUNXI_OP_SHA1;
	} else {
		tmp |= SUNXI_OP_MD5;
		op->mode = SUNXI_OP_MD5;
	}

	writel(tmp, ss_ctx->base + SUNXI_SS_CTL);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
/*
 * sunxi_hash_update: update hash engine
 *
 * Could be used for both SHA1 and MD5
 * Write data by step of 32bits and put then in the SS.
 * The remaining data is stored (nbwait bytes) in op->waitbuf
 * As an optimisation, we do not check RXFIFO_SPACES, since SS handle
 * the FIFO faster than our writes
 */
static int sunxi_shash_update(struct shash_desc *desc,
		const u8 *data, unsigned int length)
{
	u32 v;
	unsigned int i = 0;
	struct sunxi_req_ctx *op = crypto_shash_ctx(desc->tfm);

	u8 *waitbuf = (u8 *)(&op->waitbuf);

	if (length == 0)
		return 0;

	if (op->nbwait > 0) {
		for (; op->nbwait < 4 && i < length; op->nbwait++) {
			waitbuf[op->nbwait] = *(data + i);
			i++;
		}
		if (op->nbwait == 4) {
			writel(op->waitbuf, ss_ctx->base + SS_RXFIFO);
			op->byte_count += 4;
			op->nbwait = 0;
			op->waitbuf = 0;
		}
	}
	/*if (length - i > 3 && ((length - i) % 4) == 0) {*/
	/* TODO bench this optim */
	if (i == 0 && ((length - i) % 4) == 0) {
		u32 *src32 = (u32 *)(data + i);
		i = (length - i) / 4;
		while (i > 0) {
			writel_relaxed(*src32++, ss_ctx->base + SS_RXFIFO);
			i--;
		}
		op->byte_count += length;
		return 0;
	}
	while (length - i >= 4) {
		v = *(u32 *)(data + i);
		writel_relaxed(v, ss_ctx->base + SS_RXFIFO);
		i += 4;
		op->byte_count += 4;
	}
	/* if we have less than 4 bytes, copy them in waitbuf */
	if (i < length && length - i < 4) {
		do {
			waitbuf[op->nbwait] = *(data + i + op->nbwait);
			op->nbwait++;
		} while (i + op->nbwait < length);
	}

	return 0;
}

/*============================================================================*/
/*============================================================================*/
/*
 * sunxi_hash_final: finalize hashing operation
 *
 * If we have some remaining bytes, send it.
 * Then ask the SS for finalizing the hash
 */
static int sunxi_shash_final(struct shash_desc *desc, u8 *out)
{
	u32 v;
	unsigned int i;
	int zeros;
	unsigned int index, padlen;
	__be64 bits;
	struct sunxi_req_ctx *op = crypto_shash_ctx(desc->tfm);

	if (op->nbwait > 0) {
		op->waitbuf |= ((1 << 7) << (op->nbwait * 8));
		writel(op->waitbuf, ss_ctx->base + SS_RXFIFO);
	} else {
		writel((1 << 7), ss_ctx->base + SS_RXFIFO);
	}

	/* number of space to pad to obtain 64o minus 8(size) minus 4 (final 1)
	 * example len=0
	 * example len=56
	 * */

	/* we have already send 4 more byte of which nbwait data */
	if (op->mode == SUNXI_OP_MD5) {
		index = (op->byte_count + 4) & 0x3f;
		op->byte_count += op->nbwait;
		if (index > 56)
			zeros = (120 - index) / 4;
		else
			zeros = (56 - index) / 4;
	} else {
		op->byte_count += op->nbwait;
		index = op->byte_count & 0x3f;
		padlen = (index < 56) ? (56 - index) : ((64+56) - index);
		zeros = (padlen - 1) / 4;
	}
#ifdef DEBUG
	/* This should not happen, TODO set a unlikely() ? */
	if (zeros > 64 || zeros < 0) {
		dev_err(ss_ctx->dev, "ERROR: too many zeros len=%llu\n",
			op->byte_count);
		zeros = 0;
	}
#endif
	for (i = 0; i < zeros; i++)
		writel(0, ss_ctx->base + SS_RXFIFO);

	/* write the lenght */
	if (op->mode == SUNXI_OP_SHA1) {
		bits = cpu_to_be64(op->byte_count << 3);
		writel(bits & 0xffffffff, ss_ctx->base + SS_RXFIFO);
		writel((bits >> 32) & 0xffffffff,
		       ss_ctx->base + SS_RXFIFO);
	} else {
		writel((op->byte_count << 3) & 0xffffffff,
				ss_ctx->base + SS_RXFIFO);
		writel((op->byte_count >> 29) & 0xffffffff,
				ss_ctx->base + SS_RXFIFO);
	}

	/* stop the hashing */
	v = readl(ss_ctx->base + SUNXI_SS_CTL);
	v |= SUNXI_SS_DATA_END;
	writel(v, ss_ctx->base + SUNXI_SS_CTL);

	/* check the end */
	/* The timeout could happend only in case of bad overcloking */
#define SUNXI_SS_TIMEOUT 100
	i = 0;
	do {
		v = readl(ss_ctx->base + SUNXI_SS_CTL);
		i++;
	} while (i < SUNXI_SS_TIMEOUT && (v & SUNXI_SS_DATA_END) > 0);
	if (i >= SUNXI_SS_TIMEOUT) {
		dev_err(ss_ctx->dev, "SUNXI_SS_TIMEOUT %d>%d\n",
				i, SUNXI_SS_TIMEOUT);
		writel(0, ss_ctx->base + SUNXI_SS_CTL);
		mutex_unlock(&lock);
		return -1;
	}

	if (op->mode == SUNXI_OP_SHA1) {
		for (i = 0; i < 5; i++) {
			v = cpu_to_be32(readl(ss_ctx->base +
						SUNXI_SS_MD0 + i * 4));
			memcpy(out + i * 4, &v, 4);
		}
	} else {
		for (i = 0; i < 4; i++) {
			v = readl(ss_ctx->base + SUNXI_SS_MD0 + i * 4);
			memcpy(out + i * 4, &v, 4);
		}
	}
	writel(0, ss_ctx->base + SUNXI_SS_CTL);
	mutex_unlock(&lock);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
/* sunxi_hash_finup: finalize hashing operation after an update */
static int sunxi_shash_finup(struct shash_desc *desc, const u8 *in,
		unsigned int count, u8 *out)
{
	int err;

	err = sunxi_shash_update(desc, in, count);
	if (err != 0)
		return err;

	return sunxi_shash_final(desc, out);
}

/*============================================================================*/
/*============================================================================*/
/* combo of init/update/final functions */
static int sunxi_shash_digest(struct shash_desc *desc, const u8 *in,
		unsigned int count, u8 *out)
{
	int err;

	err = sunxi_shash_init(desc);
	if (err != 0)
		return err;

	err = sunxi_shash_update(desc, in, count);
	if (err != 0)
		return err;

	return sunxi_shash_final(desc, out);
}

/*============================================================================*/
/*============================================================================*/
static struct shash_alg sunxi_md5_alg = {
	.init = sunxi_shash_init,
	.update = sunxi_shash_update,
	.final = sunxi_shash_final,
	.finup = sunxi_shash_finup,
	.digest = sunxi_shash_digest,
	.digestsize = MD5_DIGEST_SIZE,
	.base = {
		.cra_name = "md5",
		.cra_driver_name = "md5-sunxi-ss",
		.cra_priority = 300,
		.cra_alignmask = 3,
		.cra_flags = CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize = MD5_HMAC_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct sunxi_req_ctx),
		.cra_module = THIS_MODULE,
	}
};

static struct shash_alg sunxi_sha1_alg = {
	.init = sunxi_shash_init,
	.update = sunxi_shash_update,
	.final = sunxi_shash_final,
	.finup = sunxi_shash_finup,
	.digest = sunxi_shash_digest,
	.digestsize = SHA1_DIGEST_SIZE,
	.base = {
		.cra_name = "sha1",
		.cra_driver_name = "sha1-sunxi-ss",
		.cra_priority = 300,
		.cra_alignmask = 3,
		.cra_flags = CRYPTO_ALG_TYPE_SHASH,
		.cra_ctxsize = sizeof(struct sunxi_req_ctx),
		.cra_module = THIS_MODULE,
	}
};
#endif /* ifdef SUNXI_SS_HASH_COMMON */

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_AES
/*============================================================================*/
/*============================================================================*/
static int sunxi_aes_poll(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		const unsigned int nbytes, const u32 flag)
{
	u32 tmp;
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	u32 rx_cnt = 32; /* when activating SS, the default FIFO space is 32 */
	u32 tx_cnt = 0;
	u32 v;
	int i;
	struct scatterlist *in_sg;
	struct scatterlist *out_sg;
	void *src_addr;
	void *dst_addr;
	unsigned int ileft = nbytes;
	unsigned int oleft = nbytes;
	unsigned int sgileft = src->length;
	unsigned int sgoleft = dst->length;
	unsigned int todo;
	u32 *src32;
	u32 *dst32;

	tmp = flag;
	tmp |= SUNXI_SS_KEYSELECT_KEYN;
	tmp |= SUNXI_SS_ENABLED;

	in_sg = src;
	out_sg = dst;
	if (src == NULL || dst == NULL) {
		dev_err(ss_ctx->dev, "Some SGs are NULL\n");
		return -1;
	}
	mutex_lock(&lock);
	if (desc->info != NULL) {
		for (i = 0; i < op->keylen; i += 4) {
			v = *(u32 *)(op->key + i);
			writel(v, ss_ctx->base + SUNXI_SS_KEY0 + i);
		}
		for (i = 0; i < 4; i++) {
			v = *(u32 *)(desc->info + i * 4);
			writel(v, ss_ctx->base + SUNXI_SS_IV0 + i * 4);
		}
	}
	writel(tmp, ss_ctx->base + SUNXI_SS_CTL);

	/* If we have only one SG, we can use kmap_atomic */
	if (sg_next(in_sg) == NULL && sg_next(out_sg) == NULL) {
		src_addr = kmap_atomic(sg_page(in_sg)) + in_sg->offset;
		if (src_addr == NULL) {
			dev_err(ss_ctx->dev, "kmap_atomic error for src SG\n");
			writel(0, ss_ctx->base + SUNXI_SS_CTL);
			mutex_unlock(&lock);
			return -1;
		}
		dst_addr = kmap_atomic(sg_page(out_sg)) + out_sg->offset;
		if (dst_addr == NULL) {
			dev_err(ss_ctx->dev, "kmap_atomic error for dst SG\n");
			writel(0, ss_ctx->base + SUNXI_SS_CTL);
			mutex_unlock(&lock);
			kunmap_atomic(src_addr);
			return -1;
		}
		src32 = (u32 *)src_addr;
		dst32 = (u32 *)dst_addr;
		ileft = nbytes / 4;
		oleft = nbytes / 4;
		do {
			if (ileft > 0 && rx_cnt > 0) {
				todo = min(rx_cnt, ileft);
				ileft -= todo;
				do {
					writel_relaxed(*src32++,
						       ss_ctx->base +
						       SS_RXFIFO);
					todo--;
				} while (todo > 0);
			}
			if (tx_cnt > 0) {
				todo = min(tx_cnt, oleft);
				oleft -= todo;
				do {
					*dst32++ = readl_relaxed(ss_ctx->base +
								SS_TXFIFO);
					todo--;
				} while (todo > 0);
			}
			tmp = readl_relaxed(ss_ctx->base + SUNXI_SS_FCSR);
			rx_cnt = SS_RXFIFO_SPACES(tmp);
			tx_cnt = SS_TXFIFO_SPACES(tmp);
		} while (oleft > 0);
		writel(0, ss_ctx->base + SUNXI_SS_CTL);
		mutex_unlock(&lock);
		kunmap_atomic(src_addr);
		kunmap_atomic(dst_addr);
		return 0;
	}

	/* If we have more than one SG, we cannot use kmap_atomic since
	 * we hold the mapping too long*/
	src_addr = kmap(sg_page(in_sg)) + in_sg->offset;
	if (src_addr == NULL) {
		dev_err(ss_ctx->dev, "KMAP error for src SG\n");
		return -1;
	}
	dst_addr = kmap(sg_page(out_sg)) + out_sg->offset;
	if (dst_addr == NULL) {
		kunmap(src_addr);
		dev_err(ss_ctx->dev, "KMAP error for dst SG\n");
		return -1;
	}
	src32 = (u32 *)src_addr;
	dst32 = (u32 *)dst_addr;
	ileft = nbytes / 4;
	oleft = nbytes / 4;
	sgileft = in_sg->length / 4;
	sgoleft = out_sg->length / 4;
	do {
		tmp = readl_relaxed(ss_ctx->base + SUNXI_SS_FCSR);
		rx_cnt = SS_RXFIFO_SPACES(tmp);
		tx_cnt = SS_TXFIFO_SPACES(tmp);
		todo = min3(rx_cnt, ileft, sgileft);
		if (todo > 0) {
			ileft -= todo;
			sgileft -= todo;
		}
		while (todo > 0) {
			writel_relaxed(*src32++, ss_ctx->base + SS_RXFIFO);
			todo--;
		}
		if (in_sg != NULL && sgileft == 0) {
			kunmap(sg_page(in_sg));
			in_sg = sg_next(in_sg);
			if (in_sg != NULL && ileft > 0) {
				src_addr = kmap(sg_page(in_sg)) + in_sg->offset;
				if (src_addr == NULL) {
					dev_err(ss_ctx->dev, "KMAP error for src SG\n");
					return -1;
				}
				src32 = src_addr;
				sgileft = in_sg->length / 4;
			}
		}
		/* do not test oleft since when oleft == 0 we have finished */
		todo = min3(tx_cnt, oleft, sgoleft);
		if (todo > 0) {
			oleft -= todo;
			sgoleft -= todo;
		}
		while (todo > 0) {
			*dst32++ = readl_relaxed(ss_ctx->base + SS_TXFIFO);
			todo--;
		}
		if (out_sg != NULL && sgoleft == 0) {
			kunmap(sg_page(out_sg));
			out_sg = sg_next(out_sg);
			if (out_sg != NULL) {
				dst_addr = kmap(sg_page(out_sg)) +
					out_sg->offset;
				if (dst_addr == NULL) {
					dev_err(ss_ctx->dev, "KMAP error\n");
					return -1;
				}
				dst32 = dst_addr;
				sgoleft = out_sg->length / 4;
			}
		}
	} while (oleft > 0);

	writel(0, ss_ctx->base + SUNXI_SS_CTL);
	mutex_unlock(&lock);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_aes_cbc_encrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (unlikely(ivsize < 4)) {
		dev_err(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_err(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_ENCRYPTION;
	op->mode |= SUNXI_OP_AES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_aes_poll(desc, dst, src, nbytes, op->mode);
}
/*============================================================================*/
/*============================================================================*/
static int sunxi_aes_cbc_decrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (unlikely(ivsize < 4)) {
		dev_err(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_err(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_DECRYPTION;
	op->mode |= SUNXI_OP_AES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_aes_poll(desc, dst, src, nbytes, op->mode);
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_aes_init(struct crypto_tfm *tfm)
{
	struct sunxi_req_ctx *op = crypto_tfm_ctx(tfm);
	memset(op, 0, sizeof(struct sunxi_req_ctx));
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static void sunxi_aes_exit(struct crypto_tfm *tfm)
{
}

/*============================================================================*/
/*============================================================================*/
/* check and set the AES key, prepare the mode to be used */
static int sunxi_aes_setkey(struct crypto_tfm *tfm, const u8 *key,
		unsigned int keylen)
{
	struct sunxi_req_ctx *op = crypto_tfm_ctx(tfm);
	switch (keylen) {
	case 128 / 8:
		op->mode = SUNXI_AES_128BITS;
		break;
	case 192 / 8:
		op->mode = SUNXI_AES_192BITS;
		break;
	case 256 / 8:
		op->mode = SUNXI_AES_256BITS;
		break;
	default:
		dev_err(ss_ctx->dev, "Invalid keylen %u\n", keylen);
		crypto_tfm_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static struct crypto_alg sunxi_aes_alg = {
	.cra_name = "cbc(aes)",
	.cra_driver_name = "cbc-aes-sunxi-ss",
	.cra_priority = 300,
	.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize   = AES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sunxi_req_ctx),
	.cra_module = THIS_MODULE,
	.cra_alignmask = 3,
	.cra_type = &crypto_blkcipher_type,
	.cra_init = sunxi_aes_init,
	.cra_exit = sunxi_aes_exit,
	.cra_u = {
		.blkcipher = {
			.min_keysize    = AES_MIN_KEY_SIZE,
			.max_keysize    = AES_MAX_KEY_SIZE,
			.ivsize         = AES_BLOCK_SIZE,
			.setkey         = sunxi_aes_setkey,
			.encrypt        = sunxi_aes_cbc_encrypt,
			.decrypt        = sunxi_aes_cbc_decrypt,
		}
	}
};

#endif /* CONFIG_CRYPTO_DEV_SUNXI_SS_AES */

#ifdef SUNXI_SS_DES
/*============================================================================*/
/*============================================================================*/
/* common for DES/3DES */
static int sunxi_des_init(struct crypto_tfm *tfm)
{
	struct sunxi_req_ctx *op = crypto_tfm_ctx(tfm);
	memset(op, 0, sizeof(struct sunxi_req_ctx));
	return 0;
}

/*============================================================================*/
/*============================================================================*/
/* common for DES/3DES */
static void sunxi_des_exit(struct crypto_tfm *tfm)
{
}
/*============================================================================*/
/*============================================================================*/
/* Pure CPU way of doing DES/3DES with SS
 * Since DES and 3DES SGs could be smaller than 4 bytes, I use sg_copy_to_buffer
 * for "linearize" them.
 * The only problem with that is that I alloc (2 x nbytes) for buf_in/buf_out
 * TODO change this system
 * SGsrc -> buf_in -> SS -> buf_out -> SGdst */
static int sunxi_des_poll(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		const unsigned int nbytes, const u32 flag)
{
	u32 tmp, value;
	size_t nb_in_sg_tx, nb_in_sg_rx;
	size_t ir, it;
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	u32 tx_cnt = 0;
	u32 rx_cnt = 0;
	u32 v;
	int i;

	tmp = flag;
	tmp |= SUNXI_SS_KEYSELECT_KEYN;
	tmp |= SUNXI_SS_ENABLED;

	nb_in_sg_rx = sg_nents(src);
	nb_in_sg_tx = sg_nents(dst);

	mutex_lock(&bufin_lock);
	if (ss_ctx->buf_in == NULL) {
		ss_ctx->buf_in = kmalloc(nbytes, GFP_KERNEL);
		ss_ctx->buf_in_size = nbytes;
	} else {
		if (nbytes > ss_ctx->buf_in_size) {
			kfree(ss_ctx->buf_in);
			ss_ctx->buf_in = kmalloc(nbytes, GFP_KERNEL);
			ss_ctx->buf_in_size = nbytes;
		}
	}
	if (ss_ctx->buf_in == NULL) {
		ss_ctx->buf_in_size = 0;
		mutex_unlock(&bufin_lock);
		dev_err(ss_ctx->dev, "Unable to allocate pages.\n");
		return -ENOMEM;
	}
	if (ss_ctx->buf_out == NULL) {
		mutex_lock(&bufout_lock);
		ss_ctx->buf_out = kmalloc(nbytes, GFP_KERNEL);
		if (ss_ctx->buf_out == NULL) {
			ss_ctx->buf_out_size = 0;
			mutex_unlock(&bufout_lock);
			dev_err(ss_ctx->dev, "Unable to allocate pages.\n");
			return -ENOMEM;
		}
		ss_ctx->buf_out_size = nbytes;
		mutex_unlock(&bufout_lock);
	} else {
		if (nbytes > ss_ctx->buf_out_size) {
			mutex_lock(&bufout_lock);
			kfree(ss_ctx->buf_out);
			ss_ctx->buf_out = kmalloc(nbytes, GFP_KERNEL);
			if (ss_ctx->buf_out == NULL) {
				ss_ctx->buf_out_size = 0;
				mutex_unlock(&bufout_lock);
				dev_err(ss_ctx->dev, "Unable to allocate pages.\n");
				return -ENOMEM;
			}
			ss_ctx->buf_out_size = nbytes;
			mutex_unlock(&bufout_lock);
		}
	}

	sg_copy_to_buffer(src, nb_in_sg_rx, ss_ctx->buf_in, nbytes);

	ir = 0;
	it = 0;
	mutex_lock(&lock);
	if (desc->info != NULL) {
		for (i = 0; i < op->keylen; i += 4) {
			v = *(u32 *)(op->key + i);
			writel(v, ss_ctx->base + SUNXI_SS_KEY0 + i);
		}
		for (i = 0; i < 4; i++) {
			v = *(u32 *)(desc->info + i * 4);
			writel(v, ss_ctx->base + SUNXI_SS_IV0 + i * 4);
		}
	}
	writel(tmp, ss_ctx->base + SUNXI_SS_CTL);

	do {
		if (rx_cnt == 0 || tx_cnt == 0) {
			tmp = readl(ss_ctx->base + SUNXI_SS_FCSR);
			rx_cnt = SS_RXFIFO_SPACES(tmp);
			tx_cnt = SS_TXFIFO_SPACES(tmp);
		}
		if (rx_cnt > 0 && ir < nbytes) {
			do {
				value = *(u32 *)(ss_ctx->buf_in + ir);
				writel(value, ss_ctx->base + SS_RXFIFO);
				ir += 4;
				rx_cnt--;
			} while (rx_cnt > 0 && ir < nbytes);
		}
		if (tx_cnt > 0 && it < nbytes) {
			do {
				if (ir <= it)
					dev_warn(ss_ctx->dev, "ANORMAL %u %u\n",
							ir, it);
				value = readl(ss_ctx->base + SS_TXFIFO);
				*(u32 *)(ss_ctx->buf_out + it) = value;
				it += 4;
				tx_cnt--;
			} while (tx_cnt > 0 && it < nbytes);
		}
		if (ir == nbytes) {
			mutex_unlock(&bufin_lock);
			ir++;
		}
	} while (it < nbytes);

	writel(0, ss_ctx->base + SUNXI_SS_CTL);
	mutex_unlock(&lock);

	/* a simple optimization, since we dont need the hardware for this copy
	 * we release the lock and do the copy. With that we gain 5/10% perf */
	mutex_lock(&bufout_lock);
	sg_copy_from_buffer(dst, nb_in_sg_tx, ss_ctx->buf_out, nbytes);

	mutex_unlock(&bufout_lock);
	return 0;
}
#endif

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_DES
/*============================================================================*/
/*============================================================================*/
/* check and set the DES key, prepare the mode to be used */
static int sunxi_des_setkey(struct crypto_tfm *tfm, const u8 *key,
		unsigned int keylen)
{
	struct sunxi_req_ctx *op = crypto_tfm_ctx(tfm);
	if (keylen != DES_KEY_SIZE) {
		dev_err(ss_ctx->dev, "Invalid keylen %u\n", keylen);
		crypto_tfm_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_des_cbc_encrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (ivsize < 4) {
		dev_info(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_info(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_ENCRYPTION;
	op->mode |= SUNXI_OP_DES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_des_poll(desc, dst, src, nbytes, op->mode);
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_des_cbc_decrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (ivsize < 4) {
		dev_info(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_info(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_DECRYPTION;
	op->mode |= SUNXI_OP_DES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_des_poll(desc, dst, src, nbytes, op->mode);
}

/*============================================================================*/
/*============================================================================*/
static struct crypto_alg sunxi_des_alg = {
	.cra_name = "cbc(des)",
	.cra_driver_name = "cbc-des-sunxi-ss",
	.cra_priority = 300,
	.cra_blocksize = DES_BLOCK_SIZE,
	.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_ctxsize = sizeof(struct sunxi_req_ctx),
	.cra_module = THIS_MODULE,
	.cra_type = &crypto_blkcipher_type,
	.cra_init = sunxi_des_init,
	.cra_exit = sunxi_des_exit,
	.cra_alignmask = 3,
	.cra_u.blkcipher = {
		.min_keysize    = DES_KEY_SIZE,
		.max_keysize    = DES_KEY_SIZE,
		.ivsize         = 8,
		.setkey         = sunxi_des_setkey,
		.encrypt        = sunxi_des_cbc_encrypt,
		.decrypt        = sunxi_des_cbc_decrypt,
	}
};

#endif /* CONFIG_CRYPTO_DEV_SUNXI_SS_DES */

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_3DES
/*============================================================================*/
/*============================================================================*/
/* check and set the 3DES key, prepare the mode to be used */
static int sunxi_des3_setkey(struct crypto_tfm *tfm, const u8 *key,
		unsigned int keylen)
{
	struct sunxi_req_ctx *op = crypto_tfm_ctx(tfm);
	if (keylen != 3 * DES_KEY_SIZE) {
		dev_err(ss_ctx->dev, "Invalid keylen %u\n", keylen);
		crypto_tfm_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_des3_cbc_encrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (ivsize < 4) {
		dev_info(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_info(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_ENCRYPTION;
	op->mode |= SUNXI_OP_3DES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_des_poll(desc, dst, src, nbytes, op->mode);
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_des3_cbc_decrypt(struct blkcipher_desc *desc,
		struct scatterlist *dst, struct scatterlist *src,
		unsigned int nbytes)
{
	struct sunxi_req_ctx *op = crypto_blkcipher_ctx(desc->tfm);
	unsigned int ivsize = crypto_blkcipher_ivsize(desc->tfm);

	if (ivsize < 4) {
		dev_info(ss_ctx->dev, "Bad IV size %u\n", ivsize);
		return -1;
	}

	if (desc->info == NULL) {
		dev_info(ss_ctx->dev, "Empty IV\n");
		return -1;
	}

	op->mode |= SUNXI_SS_DECRYPTION;
	op->mode |= SUNXI_OP_3DES;
	op->mode |= SUNXI_SS_CBC;

	return sunxi_des_poll(desc, dst, src, nbytes, op->mode);
}

/*============================================================================*/
/*============================================================================*/
static struct crypto_alg sunxi_des3_alg = {
	.cra_name = "cbc(des3_ede)",
	.cra_driver_name = "cbc-des3-sunxi-ss",
	.cra_priority = 300,
	.cra_blocksize = DES3_EDE_BLOCK_SIZE,
	.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_ctxsize = sizeof(struct sunxi_req_ctx),
	.cra_module = THIS_MODULE,
	.cra_type = &crypto_blkcipher_type,
	.cra_init = sunxi_des_init,
	.cra_exit = sunxi_des_exit,
	.cra_alignmask = 3,
	.cra_u.blkcipher = {
		.min_keysize    = DES3_EDE_KEY_SIZE,
		.max_keysize    = DES3_EDE_KEY_SIZE,
		.ivsize         = 8,
		.setkey         = sunxi_des3_setkey,
		.encrypt        = sunxi_des3_cbc_encrypt,
		.decrypt        = sunxi_des3_cbc_decrypt,
	}
};

#endif /* CONFIG_CRYPTO_DEV_SUNXI_SS_3DES */

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_PRNG
/*============================================================================*/
/*============================================================================*/
static int sunxi_ss_rng_get_random(struct crypto_rng *tfm, u8 *rdata,
		unsigned int dlen)
{
	struct prng_context *ctx = crypto_tfm_ctx((struct crypto_tfm *)tfm);
	unsigned int i;
	u32 mode = 0;
	u32 v;

	dev_dbg(ss_ctx->dev, "DEBUG %s dlen=%u\n", __func__, dlen);

	if (dlen == 0 || rdata == NULL)
		return 0;

	mode |= SUNXI_OP_PRNG;
	mode |= SUNXI_PRNG_ONESHOT;
	mode |= SUNXI_SS_ENABLED;

	mutex_lock(&lock);
	writel(mode, ss_ctx->base + SUNXI_SS_CTL);

	for (i = 0; i < ctx->slen; i += 4) {
		v = *(u32 *)(ctx->seed + i);
		dev_dbg(ss_ctx->dev, "DEBUG Seed %d %x\n", i, v);
	}

	for (i = 0; i < ctx->slen && i < 192/8 && i < 16; i += 4) {
		v = *(u32 *)(ctx->seed + i);
		dev_dbg(ss_ctx->dev, "DEBUG Seed %d %x\n", i, v);
		writel(v, ss_ctx->base + SUNXI_SS_KEY0 + i);
	}

	mode |= SUNXI_PRNG_START;
	writel(mode, ss_ctx->base + SUNXI_SS_CTL);
	for (i = 0; i < 4; i++) {
		v = readl(ss_ctx->base + SUNXI_SS_CTL);
		dev_dbg(ss_ctx->dev, "DEBUG CTL %x %x\n", mode, v);
	}
	for (i = 0; i < dlen && i < 160 / 8; i += 4) {
		v = readl(ss_ctx->base + SUNXI_SS_MD0 + i);
		*(u32 *)(rdata + i) = v;
		dev_dbg(ss_ctx->dev, "DEBUG MD%d %x\n", i, v);
	}

	writel(0, ss_ctx->base + SUNXI_SS_CTL);
	mutex_unlock(&lock);
	return dlen;
}

/*============================================================================*/
/*============================================================================*/
static int sunxi_ss_rng_reset(struct crypto_rng *tfm, u8 *seed,
		unsigned int slen)
{
	struct prng_context *ctx = crypto_tfm_ctx((struct crypto_tfm *)tfm);

	dev_dbg(ss_ctx->dev, "DEBUG %s slen=%u\n", __func__, slen);
	memcpy(ctx->seed, seed, slen);
	ctx->slen = slen;
	return 0;
}

/*============================================================================*/
/*============================================================================*/
/* TODO perhaps we could implement ansi_cprng ? */
static struct crypto_alg sunxi_ss_prng = {
	.cra_name = "stdrng",
	.cra_driver_name = "rng-sunxi-ss",
	.cra_priority = 100,
	.cra_flags = CRYPTO_ALG_TYPE_RNG,
	.cra_ctxsize = sizeof(struct prng_context),
	.cra_module = THIS_MODULE,
	.cra_type = &crypto_rng_type,
	.cra_u.rng = {
		.rng_make_random = sunxi_ss_rng_get_random,
		.rng_reset = sunxi_ss_rng_reset,
		.seedsize = 192/8
	}
};
#endif /* CRYPTO_DEV_SUNXI_SS_PRNG */

/*============================================================================*/
/*============================================================================*/
static int sunxi_ss_probe(struct platform_device *pdev)
{
	struct resource *res;
	u32 v;
	int err;
	unsigned long cr;

	if (!pdev->dev.of_node)
		return -ENODEV;

	memset(ss_ctx, 0, sizeof(struct sunxi_ss_ctx));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get the MMIO ressource\n");
		/* TODO PTR_ERR ? */
		return -ENXIO;
	}
	ss_ctx->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ss_ctx->base)) {
		dev_err(&pdev->dev, "Cannot request MMIO\n");
		return PTR_ERR(ss_ctx->base);
	}

	/* TODO Does this information could be usefull ? */
	writel(SUNXI_SS_ENABLED, ss_ctx->base + SUNXI_SS_CTL);
	v = readl(ss_ctx->base + SUNXI_SS_CTL);
	v >>= 16;
	v &= 0x07;
	dev_info(&pdev->dev, "Die ID %d\n", v);
	writel(0, ss_ctx->base + SUNXI_SS_CTL);

	ss_ctx->ssclk = devm_clk_get(&pdev->dev, "mod");
	if (IS_ERR(ss_ctx->ssclk)) {
		err = PTR_ERR(ss_ctx->ssclk);
		dev_err(&pdev->dev, "Cannot get SS clock err=%d\n", err);
		return err;
	}
	dev_dbg(&pdev->dev, "clock ss acquired\n");

	ss_ctx->busclk = devm_clk_get(&pdev->dev, "ahb");
	if (IS_ERR(ss_ctx->busclk)) {
		err = PTR_ERR(ss_ctx->busclk);
		dev_err(&pdev->dev, "Cannot get AHB SS clock err=%d\n", err);
		return err;
	}
	dev_dbg(&pdev->dev, "clock ahb_ss acquired\n");


	/* Enable the clocks */
	err = clk_prepare_enable(ss_ctx->busclk);
	if (err != 0) {
		dev_err(&pdev->dev, "Cannot prepare_enable busclk\n");
		return err;
	}
	err = clk_prepare_enable(ss_ctx->ssclk);
	if (err != 0) {
		dev_err(&pdev->dev, "Cannot prepare_enable ssclk\n");
		clk_disable_unprepare(ss_ctx->busclk);
		return err;
	}

#define	SUNXI_SS_CLOCK_RATE_BUS (24 * 1000 * 1000)
#define	SUNXI_SS_CLOCK_RATE_SS (150 * 1000 * 1000)

	/* Check that clock have the correct rates gived in the datasheet */
	cr = clk_get_rate(ss_ctx->busclk);
	if (cr >= SUNXI_SS_CLOCK_RATE_BUS)
		dev_dbg(&pdev->dev, "Clock bus %lu (%lu MHz) (must be >= %u)\n",
				cr, cr / 1000000, SUNXI_SS_CLOCK_RATE_BUS);
	else
		dev_warn(&pdev->dev, "Clock bus %lu (%lu MHz) (must be >= %u)\n",
				cr, cr / 1000000, SUNXI_SS_CLOCK_RATE_BUS);
	cr = clk_get_rate(ss_ctx->ssclk);
	if (cr == SUNXI_SS_CLOCK_RATE_SS)
		dev_dbg(&pdev->dev, "Clock ss %lu (%lu MHz) (must be <= %u)\n",
				cr, cr / 1000000, SUNXI_SS_CLOCK_RATE_SS);
	else {
		dev_warn(&pdev->dev, "Clock ss is at %lu (%lu MHz) (must be <= %u)\n",
				cr, cr / 1000000, SUNXI_SS_CLOCK_RATE_SS);
		/* Try to set the clock to the maximum allowed */
		err = clk_set_rate(ss_ctx->ssclk, SUNXI_SS_CLOCK_RATE_SS);
		if (err != 0) {
			dev_err(&pdev->dev, "Cannot set clock rate to ssclk\n");
			goto label_error_clock;
		}
		cr = clk_get_rate(ss_ctx->ssclk);
		dev_info(&pdev->dev, "Clock ss set to %lu (%lu MHz) (must be >= %u)\n",
				cr, cr / 1000000, SUNXI_SS_CLOCK_RATE_BUS);
	}

	ss_ctx->buf_in = NULL;
	ss_ctx->buf_in_size = 0;
	ss_ctx->buf_out = NULL;
	ss_ctx->buf_out_size = 0;
	ss_ctx->dev = &pdev->dev;

	mutex_init(&lock);

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_PRNG
	err = crypto_register_alg(&sunxi_ss_prng);
	if (err) {
		dev_err(&pdev->dev, "crypto_register_alg error\n");
		goto label_error_prng;
	} else {
		dev_info(&pdev->dev, "Registred PRNG\n");
	}
#endif

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_MD5
	err = crypto_register_shash(&sunxi_md5_alg);
	if (err) {
		dev_err(&pdev->dev, "Fail to register MD5\n");
		goto label_error_md5;
	} else {
		dev_info(&pdev->dev, "Registred MD5\n");
	}
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_SHA1
	err = crypto_register_shash(&sunxi_sha1_alg);
	if (err) {
		dev_err(&pdev->dev, "Fail to register SHA1\n");
		goto label_error_sha1;
	} else {
		dev_info(&pdev->dev, "Registred SHA1\n");
	}
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_AES
	err = crypto_register_alg(&sunxi_aes_alg);
	if (err) {
		dev_err(&pdev->dev, "crypto_register_alg error for AES\n");
		goto label_error_aes;
	} else {
		dev_info(&pdev->dev, "Registred AES\n");
	}
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_DES
	err = crypto_register_alg(&sunxi_des_alg);
	if (err) {
		dev_err(&pdev->dev, "crypto_register_alg error for DES\n");
		goto label_error_des;
	} else {
		dev_info(&pdev->dev, "Registred DES\n");
	}
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_3DES
	err = crypto_register_alg(&sunxi_des3_alg);
	if (err) {
		dev_err(&pdev->dev, "crypto_register_alg error for 3DES\n");
		goto label_error_des3;
	} else {
		dev_info(&pdev->dev, "Registred 3DES\n");
	}
#endif
	return 0;

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_3DES
label_error_des3:
	crypto_unregister_alg(&sunxi_des3_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_DES
label_error_des:
	crypto_unregister_alg(&sunxi_des_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_AES
label_error_aes:
	crypto_unregister_alg(&sunxi_aes_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_SHA1
label_error_sha1:
	crypto_unregister_shash(&sunxi_sha1_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_MD5
label_error_md5:
	crypto_unregister_shash(&sunxi_md5_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_PRNG
label_error_prng:
	crypto_unregister_alg(&sunxi_ss_prng);
#endif
label_error_clock:
	if (ss_ctx->ssclk != NULL)
		clk_disable_unprepare(ss_ctx->ssclk);
	if (ss_ctx->busclk != NULL)
		clk_disable_unprepare(ss_ctx->busclk);

	return err;
}

/*============================================================================*/
/*============================================================================*/
static int __exit sunxi_ss_remove(struct platform_device *pdev)
{
	if (!pdev->dev.of_node)
		return 0;

#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_MD5
	crypto_unregister_shash(&sunxi_md5_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_SHA1
	crypto_unregister_shash(&sunxi_sha1_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_PRNG
	crypto_unregister_alg(&sunxi_ss_prng);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_AES
	crypto_unregister_alg(&sunxi_aes_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_3DES
	crypto_unregister_alg(&sunxi_des3_alg);
#endif
#ifdef CONFIG_CRYPTO_DEV_SUNXI_SS_DES
	crypto_unregister_alg(&sunxi_des_alg);
#endif
	/* TODO devm_kmalloc / devm_kfree */
	if (ss_ctx->buf_in != NULL)
		kfree(ss_ctx->buf_in);
	if (ss_ctx->buf_out != NULL)
		kfree(ss_ctx->buf_out);

	writel(0, ss_ctx->base + SUNXI_SS_CTL);
	clk_disable_unprepare(ss_ctx->busclk);
	clk_disable_unprepare(ss_ctx->ssclk);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
static const struct of_device_id a20ss_crypto_of_match_table[] = {
	{ .compatible = "allwinner,sun7i-a20-crypto" },
	{}
};
MODULE_DEVICE_TABLE(of, a20ss_crypto_of_match_table);

static struct platform_driver sunxi_ss_driver = {
	.probe          = sunxi_ss_probe,
	.remove         = sunxi_ss_remove,
	.driver         = {
		.owner          = THIS_MODULE,
		.name           = "sunxi-ss",
		.of_match_table	= a20ss_crypto_of_match_table,
	},
};

module_platform_driver(sunxi_ss_driver);

MODULE_DESCRIPTION("Allwinner Security System crypto accelerator");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corentin LABBE <clabbe.montjoie@gmail.com>");
