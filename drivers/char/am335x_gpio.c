/* 
	linux/drivers/char/am335x_gpio.c
	AM335x GPIO sample 
*/

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <plat/mux.h>
#include <mach/gpio.h>
#include <mach/irqs.h>

#define GPIO_FREE_IRQ	0
#define GPIO_REG_IRQ	1

#define DEV_NAME "am335x_gpio"
#define DEV_MAJOR 250
#define GPIO_TO_PIN(bank, gpio)		(32 * (bank) + gpio)
#define MAX_LIST	10

struct gpio_ioctl_st {
	int pin;
	int irq;
	int sig;
	pid_t pid;
};

struct gpio_dev_st
{
	struct cdev cdev;
	dev_t devno;
	struct class *gpio_class;
	unsigned int status;
	struct gpio_ioctl_st gpio_irqlist[MAX_LIST]; 
};

struct gpio_dev_st gpio_dev;

irqreturn_t irq_handler(int irqno, void *dev_id)
{
	struct gpio_dev_st *dev = &gpio_dev;
	struct task_struct *p = NULL;
	int idx;

	for(idx=0;idx<MAX_LIST;idx++){
		if(dev->gpio_irqlist[idx].irq == irqno)
			break;
	}
	if(idx >= MAX_LIST){
		printk("Unknow IRQ %d\n", irqno);
		return IRQ_HANDLED;
	}
	
	printk("irq_handler\n");
	p = find_task_by_vpid(dev->gpio_irqlist[idx].pid);
	if(p != NULL){
		printk("Send SIGUSR2 to user\n");
		send_sig(dev->gpio_irqlist[idx].sig, p, 1);	
	}
	return IRQ_HANDLED;
}

int gpio_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

ssize_t gpio_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

ssize_t gpio_read(struct file *file, char __user * buf, size_t len, loff_t * ppos)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

int gpio_release(struct inode *inode, struct file *filp)
{
	printk("%s\n", __FUNCTION__);
	return 0;
}

static long gpio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int irq = 0;
	int idx = 0;
	struct gpio_dev_st *dev;
	void __user *argp = (void __user *)arg;
	struct gpio_ioctl_st gpio_ioctl;
	
	copy_from_user(&gpio_ioctl, argp, sizeof(struct gpio_ioctl_st));

	printk("%s:cmd %d gpio %d\n", __FUNCTION__, cmd, gpio_ioctl.pin);

	dev = &gpio_dev;
	switch(cmd){
		case GPIO_REG_IRQ:
			break;
		case GPIO_FREE_IRQ:
			printk("Free gpio %d.\n", gpio_ioctl.pin);
			irq = gpio_to_irq(gpio_ioctl.pin);
			free_irq(irq, &dev->devno);
			gpio_free(gpio_ioctl.pin);
			for(idx=0;idx<MAX_LIST;idx++){
				if(dev->gpio_irqlist[idx].pin == gpio_ioctl.pin){
					printk("Pin %d is free\n", gpio_ioctl.pin);
					dev->gpio_irqlist[idx].pin = 255;
					dev->gpio_irqlist[idx].pid = 0;
					dev->gpio_irqlist[idx].irq = 0;
					dev->gpio_irqlist[idx].sig = 0;
					return 0;
				}
			}	
			return 0;
		default:
			printk("Unsupported command.\n");
			return 0;

	}

	if(gpio_ioctl.pin < 0 && gpio_ioctl.pin > 127){
		printk("Unsupported gpio number %d.\n", gpio_ioctl.pin);
		return 0;
	}
	
	for(idx=0;idx<MAX_LIST;idx++){
		if(dev->gpio_irqlist[idx].pin == gpio_ioctl.pin){
			printk("Pin %d is occupied\n", gpio_ioctl.pin);
			return 0;
		}
	}
	for(idx=0;idx<MAX_LIST;idx++){
		if(dev->gpio_irqlist[idx].pin == 255){
			break;
		}
	}
	if(idx >= MAX_LIST){
		printk("All interrupt Occupied\n");
		return 0;
	}

	ret = gpio_request(gpio_ioctl.pin, "gpio_intr");
	if(ret){
		printk("gpio_request for %d failed\n", gpio_ioctl.pin);
	}
	ret = gpio_direction_input(gpio_ioctl.pin);
	if(ret){
		printk("gpio_direction_input for %d failed\n", gpio_ioctl.pin);
	}
	irq = gpio_to_irq(gpio_ioctl.pin);
	if(irq < 0){
		printk("gpio_to_irq for %d failed\n", gpio_ioctl.pin);
	}
	printk("Got irq = %d\n", irq);

	ret = request_irq(irq, 
		irq_handler, 
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, 
		"gpio_intr", 
		&dev->devno);

	dev->gpio_irqlist[idx].irq = irq;
	dev->gpio_irqlist[idx].pid = gpio_ioctl.pid;
	dev->gpio_irqlist[idx].pin = gpio_ioctl.pin;
	dev->gpio_irqlist[idx].sig = gpio_ioctl.sig;

	if(ret){
		printk("request_irq() failed\n");
		gpio_free(gpio_ioctl.pin);
		dev->gpio_irqlist[idx].pin = 255;
		dev->gpio_irqlist[idx].pid = 0;
		dev->gpio_irqlist[idx].irq = 0;
		dev->gpio_irqlist[idx].sig = 0;
	}
			
	return 0;
}

struct file_operations gpio_fops =
{
		.owner = THIS_MODULE,
		.open = gpio_open,
		.write = gpio_write,
		.read = gpio_read,
		.release = gpio_release,
		.unlocked_ioctl = gpio_ioctl,
};


static int __init am335x_gpio_init(void)
{
	struct gpio_dev_st *dev;
	int i;
	

	printk(KERN_DEBUG DEV_NAME " initializing\n");

	dev = &gpio_dev;
	memset(dev, 0, sizeof(struct gpio_dev_st));

	alloc_chrdev_region(&dev->devno, 0, 1, "gpio");
	cdev_init(&dev->cdev, &gpio_fops);
	cdev_add(&dev->cdev, dev->devno, 1);
	dev->gpio_class = class_create(THIS_MODULE, "gpio_class");
	if(IS_ERR(dev->gpio_class)){
		printk(KERN_DEBUG DEV_NAME "Init Failure..!!!\n");
		device_destroy(dev->gpio_class, dev->devno);
		class_destroy(dev->gpio_class);
		cdev_del(&dev->cdev);
		unregister_chrdev_region(dev->devno, 1);
	}
	device_create(dev->gpio_class, NULL, dev->devno, NULL, "gpio");
	
	for(i=0;i<MAX_LIST; i++){
		dev->gpio_irqlist[i].pin = 255;
	}
	return 0;
}

static void __exit am335x_gpio_exit(void)
{
	struct gpio_dev_st *dev = &gpio_dev;
	
	printk(KERN_DEBUG DEV_NAME " exit\n");

	device_destroy(dev->gpio_class, dev->devno);
	class_destroy(dev->gpio_class);
	cdev_del(&dev->cdev);
	unregister_chrdev_region(dev->devno, 1);

}

module_init(am335x_gpio_init);
module_exit(am335x_gpio_exit);

MODULE_AUTHOR("Brent Wu<brent.wu@primax.com.tw>");
MODULE_DESCRIPTION("Simple AM335x GPIO module");
MODULE_LICENSE("GPL");
