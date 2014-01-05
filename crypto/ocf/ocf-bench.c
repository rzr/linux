/*
 * benchmark in kernel crypto speed
 */


#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <cryptodev.h>
#include <asm/delay.h>

#define DECC
#define BOTH
 
/*
 * the number of simultaneously active requests
 */
static int request_q_len = 20;
MODULE_PARM(request_q_len, "i");
MODULE_PARM_DESC(request_q_len, "Number of outstanding requests");
/*
 * how many requests we want to have processed
 */
static int request_num = 1024;
MODULE_PARM(request_num, "i");
MODULE_PARM_DESC(request_num, "run for at least this many requests");
/*
 * the size of each request
 */
static int request_size = 1408;
MODULE_PARM(request_size, "i");
MODULE_PARM_DESC(request_size, "size of each request");
/*
 * a structure for each request
 */
typedef struct  {
	struct work_struct work;
#ifdef DECC
	int encrypt;
	int index;
#endif
	unsigned char *buffer;
} request_t;

static request_t *requests, *requests2;

static int outstanding;
static int total;

/*************************************************************************/
/*
 * OCF benchmark routines
 */

static uint64_t ocf_cryptoid;
static int ocf_init(void);
static int ocf_cb(struct cryptop *crp);
static void ocf_request(void *arg);

void
hexdump(char *p, int n)
{
	int i, off;

	for (off = 0; n > 0; off += 16, n -= 16) {
		printk("%s%04x:", off == 0 ? "\n" : "", off);
		i = (n >= 16 ? 16 : n);
		do {
			printk(" %02x", *p++ & 0xff);
		} while (--i);
		printk("\n");
	}
}

static int
ocf_init(void)
{
	int error;
	struct cryptoini crie, cria;
	struct cryptodesc crda, crde;

	memset(&crie, 0, sizeof(crie));
	memset(&cria, 0, sizeof(cria));
	memset(&crde, 0, sizeof(crde));
	memset(&crda, 0, sizeof(crda));

#ifdef BOTH
	cria.cri_alg  = CRYPTO_MD5_HMAC;
	cria.cri_klen = 20 * 8;
	cria.cri_key  = "0123456789abcdefghij";
#endif
	crie.cri_alg  = CRYPTO_3DES_CBC;
	crie.cri_klen = 24 * 8;
	crie.cri_key  = "0123456789abcdefghijklmn";

#ifdef BOTH
	crie.cri_next = &cria;
#endif
	error = crypto_newsession(&ocf_cryptoid, &crie, 0);
	if (error) {
		printk("crypto_newsession failed %d\n", error);
		return -1;
	}
	return 0;
}

static int
ocf_cb(struct cryptop *crp)
{
	request_t *r = (request_t *) crp->crp_opaque;

	crypto_freereq(crp);
	crp = NULL;

	total++;
#ifdef DECC
	if(r->encrypt == 0)
		r->encrypt = 1;
	else
		r->encrypt = 0;

	if ((total > request_num) && r->encrypt) {
#else

	if (total > request_num) {
#endif
		outstanding--;
		return 0;
	}	

	INIT_WORK(&r->work, ocf_request, r);
	schedule_work(&r->work);
	return 0;
}


static void
ocf_request(void *arg)
{
	request_t *r = arg;
#ifdef BOTH
	struct cryptop *crp = crypto_getreq(2);
#else
	int i;
	struct cryptop *crp = crypto_getreq(1);
#endif
	struct cryptodesc *crde, *crda;
	int timeout = 0;

	if (!crp) {
		outstanding--;
		return;
	}

#ifdef BOTH
	if(r->encrypt) {
		crde = crp->crp_desc;
		crda = crde->crd_next;
	}
	else {
		crda = crp->crp_desc;
		crde = crda->crd_next;
	}

	crda->crd_skip = 0;
	crda->crd_flags = 0;
	crda->crd_len = request_size;
#ifdef DECC
	if( r->encrypt )
		crda->crd_inject = request_size + 32;
	else
		crda->crd_inject = request_size + 64;
#else
	crda->crd_inject = request_size + 32;
#endif
	crda->crd_alg = CRYPTO_MD5_HMAC;
	crda->crd_key = "0123456789abcdefghij";
	crda->crd_klen = 20 * 8;
#else /* BOTH */
	crde = crp->crp_desc;
#endif /* BOTH */
	crde->crd_skip = 0;
#ifdef DECC
	if( r->encrypt )
		crde->crd_flags = CRD_F_IV_EXPLICIT | CRD_F_ENCRYPT;
	else
		crde->crd_flags = CRD_F_IV_EXPLICIT;
#else
	crde->crd_flags = CRD_F_IV_EXPLICIT | CRD_F_ENCRYPT;
#endif

	crde->crd_len = request_size;
	memset(crde->crd_iv, 'a' ,EALG_MAX_BLOCK_LEN);
	crde->crd_inject = request_size;
	crde->crd_alg = CRYPTO_3DES_CBC;
	crde->crd_key = "0123456789abcdefghijklmn";
	crde->crd_klen = 24 * 8;

	crp->crp_ilen = request_size + 128;
	crp->crp_flags = CRYPTO_F_CBIMM;
	crp->crp_buf = (caddr_t) r->buffer;
	crp->crp_callback = ocf_cb;
	crp->crp_sid = ocf_cryptoid;
	crp->crp_opaque = (caddr_t) r;
	while(crypto_dispatch(crp) != 0)
	{
		if(timeout > 1000)
			printk("Failed to insert request \n");
		udelay(1);
		timeout++;
	}
}


int
ocfbench_init(void)
{
	int i,j, jstart, jstop;

	printk("Crypto Speed tests\n");

	requests = kmalloc(sizeof(request_t) * request_q_len, GFP_KERNEL);
	if (!requests) {
		printk("malloc failed\n");
		return -EINVAL;
	}

	if(request_size %8 != 0) {
		printk("request_size must be a multiple of 8!!! \n");
	}
	
	for (i = 0; i < request_q_len; i++) {
		/* +64 for return data */
		requests[i].buffer = kmalloc(request_size + 128, GFP_DMA);
		if (!requests[i].buffer) {
			printk("malloc failed\n");
			return -EINVAL;
		}
#ifdef DECC
		requests[i].encrypt = 1;
		requests[i].index = i;
#endif
	for(j=0; j < request_size + 128; j++)
		*(char *)(((char *)requests[i].buffer) + j ) = 0x2;

	}

#ifdef DECC
	requests2 = kmalloc(sizeof(request_t) * request_q_len, GFP_KERNEL);
	if (!requests) {
		printk("malloc failed\n");
		return -EINVAL;
	}

	for (i = 0; i < request_q_len; i++) {
		/* +64 for return data */
		requests2[i].buffer = kmalloc(request_size + 128, GFP_DMA);
		if (!requests2[i].buffer) {
			printk("malloc failed\n");
			return -EINVAL;
		}
		memcpy(requests2[i].buffer, requests[i].buffer, request_size);
	}
#endif
	/*
	 * OCF benchmark
	 */
	printk("OCF: testing ...\n");
	ocf_init();
	total = outstanding = 0;
	jstart = jiffies;
	for (i = 0; i < request_q_len; i++) {
		outstanding++;
		ocf_request(&requests[i]);
	}

	while (outstanding > 0) {
#if 0
		if(jiffies - jstart > 500){
			return -EINVAL;	
		}
#endif
		schedule();
	}
	jstop = jiffies;

	printk("OCF: %d requests of %d bytes in %d jiffies\n", total, request_size,
			jstop - jstart);

#ifdef DECC
	for (i = 0; i < request_q_len; i++) {
		if(memcmp(requests2[i].buffer, requests[i].buffer, request_size) != 0){
			printk("buf %d is differnt \n",i);
			printk("ORIGINAL: \n");
			hexdump(requests2[i].buffer, request_size);
			printk("AFTER \n");
			hexdump(requests[i].buffer, request_size);
			break;
		}
	}
	
#ifdef BOTH
	for (i = 0; i < request_q_len; i++) {
		if(memcmp(requests[i].buffer + request_size + 32, requests[i].buffer + request_size + 64, 16) != 0){
			printk("buf %d is differnt \n",i);
			printk("ORIGINAL: \n");
			hexdump(requests[i].buffer + request_size + 32, 16);
			printk("AFTER \n");
			hexdump(requests[i].buffer + request_size + 64, 16);
			break;
		}
	}
#endif
#endif

	for (i = 0; i < request_q_len; i++)
		kfree(requests[i].buffer);
	kfree(requests);

#ifdef DECC
	for (i = 0; i < request_q_len; i++)
		kfree(requests2[i].buffer);
	kfree(requests2);
#endif
	crypto_freesession(ocf_cryptoid);

	return -EINVAL; /* always fail to load so it can be re-run quickly ;-) */
}

static void __exit ocfbench_exit(void)
{
}

module_init(ocfbench_init);
module_exit(ocfbench_exit);

MODULE_LICENSE("BSD");
MODULE_AUTHOR("David McCullough <dmccullough@cyberguard.com>");
MODULE_DESCRIPTION("Benchmark various in-kernel crypto speeds");
