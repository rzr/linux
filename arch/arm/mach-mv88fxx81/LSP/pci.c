/*
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
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>
                                                                                                                             
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <asm/arch/irqs.h>

#include "mvPciIf.h"
#include "mvCpuIf.h"
#include "mvCtrlEnvSpec.h"

#undef DEBUG
#ifdef DEBUG
#	define DB(x) x
#else
#	define DB(x) 
#endif

static int __init mv_pri_map_irq(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_sec_map_irq(struct pci_dev *dev, u8 slot, u8 pin);

#define IF_NR MV_PCI_IF_MAX_IF

void __init mv_pci_preinit(void)
{
	unsigned int i;
	MV_ADDR_WIN win;

	for(i = 0; i < IF_NR; i++) 
	{
        	if (mvPciIfInit(i) != MV_OK)
        	{
                	printk("pci_init:Error calling mvPciIfInit for pciIf %d\n",i);
        	}
		/* unmask inter A/B/C/D */
		if ((i >= MV_PEX_START_IF) && (i < MV_PEX_MAX_IF + MV_PEX_START_IF))
        	{
			MV_REG_WRITE(MV_PCI_MASK_REG(i), MV_PCI_MASK_ABCD );
		}
	}

        /* remmap IO !! */
        win.baseLow = 0x0;
        win.baseHigh = 0x0;
        mvCpuIfPexRemap(PEX0_IO, &win);
#if defined (CONFIG_ARCH_MV88f1181)
        win.baseLow = 0x100000;
        win.baseHigh = 0x0;
        mvCpuIfPexRemap(PEX1_IO, &win);
#else
        win.baseLow = 0x100000;
        win.baseHigh = 0x0;
        mvCpuIfPciRemap(PCI0_IO, &win);
#endif

}

/* Currentlly the PCI config read/write are implemented as read modify write
   to 32 bit.
   TBD: adjust it to realy use 1/2/4 byte(partial) read/write, after the pex
	read config WA will be removed.
*/
static int mv_pci0_read_config(struct pci_bus *bus, unsigned int devfn, int where,
                          int size, u32 *val)
{

        MV_U32 bus_num,func,regOff,dev_no,temp;
	MV_U32 localBus;
 
	*val = 0xffffffff;
        bus_num = bus->number;
        dev_no = PCI_SLOT(devfn);
 
	/* don't return for our device */
	localBus = mvPciIfLocalBusNumGet(0);
	if((dev_no == 0) && ( bus_num == localBus))
	{
		DB(printk("PCI 0 read from our own dev return 0xffffffff \n"));
		return 0xffffffff;
	}

        func = PCI_FUNC(devfn); 
        regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

#if defined (CONFIG_ARCH_MV88f1181)
	/* WA: use only the first function of the bridge on PEX 0 Interface */
	if( (bus_num == 0) && (dev_no == 1) && (func != 0) )
	{
		DB(printk("PCI 0 read from bridge func != 0 return 0xffffffff \n"));
		return 0xffffffff;
	}
#endif
	DB(printk("PCI 0 read: bus = %x dev = %x func = %x regOff = %x ",bus_num,dev_no,func,regOff));

        temp = (u32) mvPciIfConfigRead(0, bus_num, dev_no, func, regOff);

        switch (size) {
        case 1:
                temp = (temp >>  (8*(where & 0x3))) & 0xff;
                break;
 
        case 2:
                temp = (temp >>  (8*(where & 0x2))) & 0xffff;
                break;
 
        default:
                break;
        }
	
	
	*val = temp;

	DB(printk(" got %x \n",temp));
        return 0;
}

static int mv_pci0_write_config(struct pci_bus *bus, unsigned int devfn, int where,
                           int size, u32 val)
{
        MV_U32 bus_num,func,regOff,dev_no,temp, mask , shift;
 
        bus_num = bus->number;
        dev_no = PCI_SLOT(devfn); 
        func = PCI_FUNC(devfn); 
        regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

	DB(printk("PCI 0: writing data %x size %x to bus %x dev %x func %x offs %x \n",val,size,bus_num,dev_no,func,regOff));
	if( size != 4)
	{
        	temp = (u32) mvPciIfConfigRead(0, bus_num, dev_no, func, regOff);
	}
	else
	{
		temp = val;
	}

        switch (size) {
        case 1:
		shift = (8*(where & 0x3));
		mask = 0xff;
                break;
 
        case 2:
		shift = (8*(where & 0x2));
                mask = 0xffff; 
                break;
 
        default:
		shift = 0;
		mask = 0xffffffff;
                break;
        }
	
	temp = (temp & (~(mask<<shift))) | ((val & mask) << shift);
	mvPciIfConfigWrite(0,bus_num,dev_no,func,regOff,temp);

        return 0;

}

static int mv_pci1_read_config(struct pci_bus *bus, unsigned int devfn, int where,
                          int size, u32 *val)
{

        MV_U32 bus_num,func,regOff,dev_no,temp;
	MV_U32	localBus;
 
	*val = 0xffffffff;

        bus_num = bus->number;
        dev_no = PCI_SLOT(devfn); 

	
	/* don't return for our device */
	localBus = mvPciIfLocalBusNumGet(1);
	if((dev_no == 0) && ( bus_num == localBus))
	{
		DB(printk("PCI 1 read from our own dev return 0xffffffff \n"));
		return 0xffffffff;
	}

        func = PCI_FUNC(devfn); 
        regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

	DB(printk("PCI 1 read: bus = %x dev = %x func = %x regOff = %x ",bus_num,dev_no,func,regOff));

        /*  We will scan only ourselves and the PCI slots that exist on the 
		board, because we may have a case that we have one slot that has
		a Cardbus connector, and because CardBus answers all IDsels we want
		to scan only this slot and ourseleves.
		
	*/
        {
		MV_U32 localBus,localDev,
			   firstSlotDevNum,slotsNum;


		slotsNum = mvBoardPciSlotsNumGet(mvPciRealIfNumGet(1));
		firstSlotDevNum= mvBoardFirstPciSlotDevNumGet(mvPciRealIfNumGet(1));
		localDev = mvPciIfLocalDevNumGet(1);
		localBus = mvPciIfLocalBusNumGet(1);

		if ((bus_num == localBus) &&
                     (!(((dev_no >= firstSlotDevNum) && (dev_no < firstSlotDevNum + slotsNum))||
				  (dev_no == localDev))))
		{
                    temp = 0xffffffff;
		}
                else
                {
                    temp = (u32) mvPciIfConfigRead(1, bus_num, dev_no, func, regOff);
                }
	}
        switch (size) {
        case 1:
                temp = (temp >>  (8*(where & 0x3))) & 0xff;
                break;
 
        case 2:
                temp = (temp >>  (8*(where & 0x2))) & 0xffff;
                break;
 
        default:
                break;
        }

	*val = temp;
	DB(printk(" got %x \n",temp));

        return 0;
}

static int mv_pci1_write_config(struct pci_bus *bus, unsigned int devfn, int where,
                           int size, u32 val)
{
        MV_U32 bus_num,func,regOff,dev_no,temp, mask , shift;
 
        bus_num = bus->number;
        dev_no = PCI_SLOT(devfn); 
        func = PCI_FUNC(devfn); 
        regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;
	
	DB(printk("PCI 0: writing data %x size %x to bus %x dev %x func %x offs %x \n",val,size,bus_num,dev_no,func,regOff));
	if( size != 4)
	{
        	temp = (u32) mvPciIfConfigRead(1, bus_num, dev_no, func, regOff);
	}
	else
	{
		temp = val;
	}

        switch (size) {
        case 1:
		shift = (8*(where & 0x3));
		mask = 0xff;
                break;
 
        case 2:
		shift = (8*(where & 0x2));
                mask = 0xffff; 
                break;
 
        default:
		shift = 0;
		mask = 0xffffffff;
                break;
        }
	
	temp = (temp & (~(mask<<shift))) | ((val & mask) << shift);
	mvPciIfConfigWrite(1,bus_num,dev_no,func,regOff,temp);

        return 0;


}


static struct pci_ops mv_primary_ops = {
        .read   = mv_pci0_read_config,
        .write  = mv_pci0_write_config,
};

static struct pci_ops mv_secondary_ops = {
        .read   = mv_pci1_read_config,
        .write  = mv_pci1_write_config,
};



int __init mv_pci_setup(int nr, struct pci_sys_data *sys)
{
        struct resource *res;
                                                                                                                             
        switch (nr) {
        case 0:
                sys->map_irq = mv_pri_map_irq;
                break;
 
        case 1:
                sys->map_irq = mv_sec_map_irq;
                break;
 
        default:
                return 0;
        }

	res = kmalloc(sizeof(struct resource) * 2, GFP_KERNEL);
        if (!res)
                panic("PCI: unable to alloc resources");
                                                                                                                             
        memset(res, 0, sizeof(struct resource) * 2);
                                                                                                                             
        switch (nr) {
        case 0:
                res[0].start = PEX0_IO_BASE - IO_SPACE_REMAP;
                res[0].end   = PEX0_IO_BASE - IO_SPACE_REMAP + PEX0_IO_SIZE-1;
                res[0].name  = "PCI IO Primary";
                res[0].flags = IORESOURCE_IO;
                                                                                                                             
                res[1].start = PEX0_MEM_BASE;
                res[1].end   = PEX0_MEM_BASE + PEX0_MEM_SIZE-1;
                res[1].name  = "PCI Memory Primary";
                res[1].flags = IORESOURCE_MEM;
                break;
                                                                                                                             
        case 1:
#if defined (CONFIG_ARCH_MV88f1181)
                res[0].start = PEX1_IO_BASE - IO_SPACE_REMAP;
                res[0].end   = PEX1_IO_BASE - IO_SPACE_REMAP + PEX1_IO_SIZE-1;
                res[0].name  = "PCI IO Primary";
                res[0].flags = IORESOURCE_IO;
                                                                                                                             
                res[1].start = PEX1_MEM_BASE;
                res[1].end   = PEX1_MEM_BASE + PEX1_MEM_SIZE-1;
                res[1].name  = "PCI Memory Primary";
                res[1].flags = IORESOURCE_MEM;
#elif defined (CONFIG_ARCH_MV88f5181)
                res[0].start = PCI0_IO_BASE - IO_SPACE_REMAP;
                res[0].end   = PCI0_IO_BASE - IO_SPACE_REMAP + PCI0_IO_SIZE-1;
                res[0].name  = "PCI IO Primary";
                res[0].flags = IORESOURCE_IO;
                                                                                                                             
                res[1].start = PCI0_MEM_BASE;
                res[1].end   = PCI0_MEM_BASE + PCI0_MEM_SIZE-1;
                res[1].name  = "PCI Memory Primary";
                res[1].flags = IORESOURCE_MEM;
#endif
                break;
        }
 
        if (request_resource(&ioport_resource, &res[0]))
	{	
		printk ("Request resource failed - PEX %x\n",nr);
	}
	if (request_resource(&iomem_resource, &res[1]))
	{	
		printk ("Request resource failed - PEX %x\n",nr);
	}
 
        sys->resource[0] = &res[0];
        sys->resource[1] = &res[1];
        sys->resource[2] = NULL;
        sys->io_offset   = 0x0;
 
        return 1;

}

struct pci_bus *mv_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	struct pci_ops *ops;

        if (nr)
                ops = &mv_secondary_ops;
        else
                ops = &mv_primary_ops;

	return pci_scan_bus(sys->busnr, ops, sys);
}




static int __init mv_pri_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{

        return IRQ_PEX0_INT;
}


#if defined (CONFIG_ARCH_MV88f1181)
static int __init mv_sec_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{

        return IRQ_PEX1_INT;
}

#elif defined (CONFIG_ARCH_MV88f5181)
static int __init mv_sec_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
        //printk("mv_sec_map_irq: calling mvBoardPciGpioPinGet\n");
	return mvBoardPciGpioPinGet(MV_PCI_START_IF, slot, pin) + IRQ_GPP_START; 
}
#endif

static struct hw_pci mv_pci __initdata = {
	.swizzle        	= pci_std_swizzle,
        .map_irq                = mv_pri_map_irq,
        .setup                  = mv_pci_setup,
        .nr_controllers         = IF_NR,
        .scan                   = mv_pci_scan_bus,
        .preinit                = mv_pci_preinit,
};
 
static int __init mv_pci_init(void)
{
        pci_common_init(&mv_pci);

        return 0;
}


subsys_initcall(mv_pci_init);

