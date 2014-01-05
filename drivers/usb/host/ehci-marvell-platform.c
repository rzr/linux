/*
 * Copyright (c) 2000-2002 by Dima Epshtein
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define DRIVER_VERSION "2004-Dec-29"
#define DRIVER_AUTHOR "Dima Epshtein"
#define DRIVER_DESC "USB 2.0 'Enhanced' Host Controller (EHCI) Driver"

#include <linux/config.h>

#ifdef CONFIG_USB_DEBUG
    #define DEBUG
#else
    #undef DEBUG
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/usb.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>

#include <linux/platform_device.h>
#define EHCI_IAA_JIFFIES	(HZ/100)
#define EHCI_IO_JIFFIES		(HZ/10)
#define EHCI_ASYNC_JIFFIES	(HZ/20)
#define EHCI_SHRINK_JIFFIES	(HZ/200)

//#define BUFFALO_USB_DEBUG
#ifdef BUFFALO_USB_DEBUG
	#define BIT(x)		(1 << x)
	#define fPROBE		BIT(1)
	#define fREMOVE		BIT(2)
	#define fINIT		BIT(3)
	#define fSTART		BIT(4)
	#define WATCH_FLAG	(~fPROBE | ~fREMOVE | ~fINIT | ~fSTART)
	#define KTRACE(y,x)	(if(y & WATCH_FLAG) x)
#else
	#define KTRACE(y,x)
#endif

#include "../core/hcd.h"
#include "ehci.h"

extern const struct hc_driver ehci_pci_hc_driver;

static int ehci_platform_probe(struct device *dev) 
{ 
    KTRACE(fPROBE, printk("    BUF_DBG %s> Entered.\n", __FUNCTION__));
    struct platform_device  *pdev = to_platform_device(dev);
    const struct hc_driver  *driver = &ehci_pci_hc_driver;
    int                     i, retval; 
    struct usb_hcd          *hcd = NULL;
    
    hcd = usb_create_hcd (driver, &pdev->dev, pdev->dev.bus_id);
    if (hcd == NULL) 
    { 
	KTRACE(fPROBE, printk("    BUF_DBG %s> hcd == NULL \n", __FUNCTION__));
        return -ENOMEM; 
    } 
 
    for(i=0; i<pdev->num_resources; i++)
    {
        if(pdev->resource[i].flags == IORESOURCE_IRQ)
        {
	    KTRACE(fPROBE, printk("    BUF_DBG %s> IORESOURCE_IRQ found (0x%x)\n", 
			__FUNCTION__, pdev->resource[i].start));
            hcd->irq = pdev->resource[i].start; 
        }
        else if(pdev->resource[i].flags == IORESOURCE_DMA)
        {
	    KTRACE(fPROBE, printk("    BUF_DBG %s> IORESOURCE_DMA found (0x%x)\n",
			__FUNCTION__, pdev->resource[i].start));
            hcd->regs = (void *)pdev->resource[i].start; 
        }
    }     
    retval = usb_add_hcd (hcd, hcd->irq, SA_SHIRQ);
	if (retval != 0)
    {
	KTRACE(fPROBE, printk("    BUF_DBG %s> usb_add_hcd failed. retval=%d\n", __FUNCTION__, retval));
        return -ENOMEM; 
    }    
    KTRACE(fPROBE, printk("    BUF_DBG %s> Leaving.\n", __FUNCTION__));
    return 0; 
} 
 
static int ehci_platform_remove(struct device *dev) 
{ 
    KTRACE(fREMOVE, printk("    BUF_DBG %s> Entered.\n", __FUNCTION__));
    struct usb_hcd          *hcd = dev_get_drvdata(dev); 
 
    printk("USB: ehci_platform_remove\n"); 
     
    usb_remove_hcd (hcd); 
    usb_put_hcd (hcd);
 
    KTRACE(fREMOVE, printk("    BUF_DBG %s> Leaving.\n", __FUNCTION__));
    return 0; 
} 

static struct device_driver ehci_platform_driver =  
{ 
    .name = "ehci_platform", 
    .bus = &platform_bus_type, 
    .probe = ehci_platform_probe, 
    .remove = ehci_platform_remove, 
};  


#define DRIVER_INFO	DRIVER_VERSION " " DRIVER_DESC

MODULE_DESCRIPTION (DRIVER_INFO);
MODULE_AUTHOR (DRIVER_AUTHOR);
MODULE_LICENSE ("GPL");

int ehci_platform_init (void) 
{ 
    KTRACE(fINIT, printk("  BUF_DBG %s> Entered.\n", __FUNCTION__));
    int status; 

    if (usb_disabled()) 
    { 
        printk("ehci_hcd: Warning - USB disabled\n");
        return -ENODEV; 
    }
 
    pr_debug ("%s: block sizes: qh %Zd qtd %Zd itd %Zd sitd %Zd\n",
        ehci_platform_driver.name,
        sizeof (struct ehci_qh), sizeof (struct ehci_qtd),
        sizeof (struct ehci_itd), sizeof (struct ehci_sitd));
 
    status = driver_register(&ehci_platform_driver); 

    KTRACE(fINIT, printk("  BUF_DBG %s> Leaving (status = %d)\n", __FUNCTION__, status));
    return status;
}

void ehci_platform_cleanup (void) 
{   
    driver_unregister(&ehci_platform_driver);
}
