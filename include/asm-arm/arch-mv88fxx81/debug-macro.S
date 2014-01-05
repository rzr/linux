/*  linux/include/asm-arm/arch-mv88fxx81/debug-macro.S
 *
 *  Debugging macro include header
 *
 *	written by fujita@soum.co.jp
 */
		.macro	addruart,rx
#if 0
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1			@ MMU enabled?
		moveq	\rx, #0xf1000000	@ physical(INTER_REGS_BASE)
		movne	\rx, #0xf1000000	@ virtual : same to physical
#else 
		mov	\rx, #0xf1000000
#endif
		orr	\rx, \rx, #0x12000	@ UART0(THR)
		.endm

		.macro	senduart,rd,rx
		str	\rd, [\rx, #0]
		.endm

		.macro	busyuart,rd,rx
/*
		XXX: need something ??? i don't know, but it works fine for me. 
*/
		.endm

		.macro	waituart,rd,rx
1001:		ldr	\rd, [\rx, #0x14]
		tst	\rd, #(1 << 5)
		beq	1001b
		.endm
