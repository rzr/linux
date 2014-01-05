/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_MMU_H
#define __ASM_ARCH_MMU_H

#include <linux/version.h>

/*
 * Task size: 3GB
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#define TASK_SIZE	(0xbf000000UL)
#define TASK_SIZE_26	(0x04000000UL)
#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15) */
#define TASK_SIZE	UL(0xbf000000)
#define TASK_SIZE_26	UL(0x04000000)
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15) */

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE (0x40000000)

/*
 * Page offset: 3GB
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#define PAGE_OFFSET	(0xc0000000UL)
#define PHYS_OFFSET	(0x00000000UL)
#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15) */
#define PAGE_OFFSET	UL(0xc0000000)
#define PHYS_OFFSET	UL(0x00000000)
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15) */

/*
 * the dram is contiguous
 */
#define __virt_to_phys__is_a_macro
#define __virt_to_phys(vpage) ((vpage) - PAGE_OFFSET)
#define __phys_to_virt__is_a_macro
#define __phys_to_virt(ppage) ((ppage) + PAGE_OFFSET)

/*
 * Virtual view <-> DMA view memory address translations
 * virt_to_bus: Used to translate the virtual address to an
 *              address suitable to be passed to set_dma_addr
 * bus_to_virt: Used to convert an address for DMA operations
 *              to an address that the kernel can use.
 */
#define __virt_to_bus__is_a_macro
#define __virt_to_bus(x)	(x - PAGE_OFFSET)
#define __bus_to_virt__is_a_macro
#define __bus_to_virt(x)	(x + PAGE_OFFSET)

#endif
