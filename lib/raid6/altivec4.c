/* -*- linux-c -*- ------------------------------------------------------- *
 *
 *   Copyright 2002-2004 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * raid6altivec4.c
 *
 * 4-way unrolled portable integer math RAID-6 instruction set
 *
 * This file is postprocessed using unroll.awk
 *
 * <benh> hpa: in process,
 * you can just "steal" the vec unit with enable_kernel_altivec() (but
 * bracked this with preempt_disable/enable or in a lock)
 */

#include <linux/raid/pq.h>

#ifdef CONFIG_ALTIVEC

#include <altivec.h>
#ifdef __KERNEL__
# include <asm/system.h>
# include <asm/cputable.h>
#endif

/*
 * This is the C data type to use.  We use a vector of
 * signed char so vec_cmpgt() will generate the right
 * instruction.
 */

typedef vector signed char unative_t;

#define NBYTES(x) ((vector signed char) {x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x})
#define NSIZE	sizeof(unative_t)

/*
 * The SHLBYTE() operation shifts each byte left by 1, *not*
 * rolling over into the next byte
 */
static inline __attribute_const__ unative_t SHLBYTE(unative_t v)
{
	return vec_add(v,v);
}

/*
 * The MASK() operation returns 0xFF in any byte for which the high
 * bit is 1, 0x00 for any byte for which the high bit is 0.
 */
static inline __attribute_const__ unative_t MASK(unative_t v)
{
	unative_t zv = NBYTES(0);

	/* vec_cmpgt returns a vector bool char; thus the need for the cast */
	return (unative_t)vec_cmpgt(zv, v);
}


/* This is noinline to make damned sure that gcc doesn't move any of the
   Altivec code around the enable/disable code */
static void noinline
raid6_altivec4_gen_syndrome_real(int disks, size_t bytes, void **ptrs)
{
	u8 **dptr = (u8 **)ptrs;
	u8 *p, *q;
	int d, z, z0;

	unative_t wd0, wq0, wp0, w10, w20;
	unative_t wd1, wq1, wp1, w11, w21;
	unative_t wd2, wq2, wp2, w12, w22;
	unative_t wd3, wq3, wp3, w13, w23;
	unative_t x1d = NBYTES(0x1d);

	z0 = disks - 3;		/* Highest data disk */
	p = dptr[z0+1];		/* XOR parity */
	q = dptr[z0+2];		/* RS syndrome */

	for ( d = 0 ; d < bytes ; d += NSIZE*4 ) {
		wq0 = wp0 = *(unative_t *)&dptr[z0][d+0*NSIZE];
		wq1 = wp1 = *(unative_t *)&dptr[z0][d+1*NSIZE];
		wq2 = wp2 = *(unative_t *)&dptr[z0][d+2*NSIZE];
		wq3 = wp3 = *(unative_t *)&dptr[z0][d+3*NSIZE];
		for ( z = z0-1 ; z >= 0 ; z-- ) {
			wd0 = *(unative_t *)&dptr[z][d+0*NSIZE];
			wd1 = *(unative_t *)&dptr[z][d+1*NSIZE];
			wd2 = *(unative_t *)&dptr[z][d+2*NSIZE];
			wd3 = *(unative_t *)&dptr[z][d+3*NSIZE];
			wp0 = vec_xor(wp0, wd0);
			wp1 = vec_xor(wp1, wd1);
			wp2 = vec_xor(wp2, wd2);
			wp3 = vec_xor(wp3, wd3);
			w20 = MASK(wq0);
			w21 = MASK(wq1);
			w22 = MASK(wq2);
			w23 = MASK(wq3);
			w10 = SHLBYTE(wq0);
			w11 = SHLBYTE(wq1);
			w12 = SHLBYTE(wq2);
			w13 = SHLBYTE(wq3);
			w20 = vec_and(w20, x1d);
			w21 = vec_and(w21, x1d);
			w22 = vec_and(w22, x1d);
			w23 = vec_and(w23, x1d);
			w10 = vec_xor(w10, w20);
			w11 = vec_xor(w11, w21);
			w12 = vec_xor(w12, w22);
			w13 = vec_xor(w13, w23);
			wq0 = vec_xor(w10, wd0);
			wq1 = vec_xor(w11, wd1);
			wq2 = vec_xor(w12, wd2);
			wq3 = vec_xor(w13, wd3);
		}
		*(unative_t *)&p[d+NSIZE*0] = wp0;
		*(unative_t *)&p[d+NSIZE*1] = wp1;
		*(unative_t *)&p[d+NSIZE*2] = wp2;
		*(unative_t *)&p[d+NSIZE*3] = wp3;
		*(unative_t *)&q[d+NSIZE*0] = wq0;
		*(unative_t *)&q[d+NSIZE*1] = wq1;
		*(unative_t *)&q[d+NSIZE*2] = wq2;
		*(unative_t *)&q[d+NSIZE*3] = wq3;
	}
}

static void raid6_altivec4_gen_syndrome(int disks, size_t bytes, void **ptrs)
{
	preempt_disable();
	enable_kernel_altivec();

	raid6_altivec4_gen_syndrome_real(disks, bytes, ptrs);

	preempt_enable();
}

int raid6_have_altivec(void);
#if 4 == 1
int raid6_have_altivec(void)
{
	/* This assumes either all CPUs have Altivec or none does */
# ifdef __KERNEL__
	return cpu_has_feature(CPU_FTR_ALTIVEC);
# else
	return 1;
# endif
}
#endif

const struct raid6_calls raid6_altivec4 = {
	raid6_altivec4_gen_syndrome,
	raid6_have_altivec,
	"altivecx4",
	0
};

#endif /* CONFIG_ALTIVEC */
