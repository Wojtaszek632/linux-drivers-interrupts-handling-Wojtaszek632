#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/ioctl.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/completion.h>

#include "mydriver.h"

#define MY_MAJOR 140
#define MY_MAX_MINORS 10
#define MY_FIRST_MINOR 0
#define NUMBER_LEN 32

static int my_init(void);
static void my_exit(void);

struct my_device_data {
struct cdev cdev;
/* data to record and identify the device */

};



unsigned int irqCounter;
spinlock_t lock;
struct completion irq_completion;

struct my_peripheral {
  void __iomem *regs;
};

  
  struct my_peripheral *peripheral;

struct my_device_data devs[MY_MAX_MINORS];

static int my_open(struct inode *inode, struct file *file)
{
	pr_info("K Opening the driver...\n");
	struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);
	
	unsigned int minor = MINOR(inode->i_cdev->dev);
	/* validate access to device */
	file->private_data = my_data;
	/* initialize device */
	pr_info("K Opening successfull\n");
	return 0;
}

int my_release(struct inode *inode, struct file *filp)
{
	pr_info("K Releasing the driver...\n");
	pr_info("K Releasing succesfull :D\n");
 	return 0;
}

static irqreturn_t my_irq_handler(int irq,void *priv)
{
	
	pr_info("K Interrupt! %d\n",irq);
	struct my_device_data *device_data = priv;

	spin_lock(&lock);			
        	
    irqCounter++;
	pr_info("K spin_unlock\n");

    spin_unlock(&lock);
	
	

	pr_info("K COUNT: %d\n",irqCounter);

	 // Signal the completion
    complete(&irq_completion);

	writel((u32 __force)cpu_to_le32(1), peripheral->regs + INTERRUPT_PENDING_REG);
	
	pr_info("exitingIRQ\n");
	return IRQ_HANDLED;

}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	pr_info("K Reading from the driver...\n");

	struct my_device_data *device_data = file->private_data;
	unsigned int irqCount;
	pr_info("K Wait for the completion to be signaled...\n");

	 // Wait for the completion to be signaled
    wait_for_completion(&irq_completion);

	pr_info("K Reset the completion...\n");
    // Reset the completion
    init_completion(&irq_completion);

    
	irqCount = irqCounter;

	char *data = (char*)kmalloc(11,GFP_USER);	
	 	
  	pr_info("K Value of counter: %d\n", irqCount);
	
	size_t datalen = sprintf(data,"%d\n",irqCount);
	if (count > datalen) {
		count = datalen;
	}
	
	if (copy_to_user(buf, data, count)) {return -EFAULT;}
	
	pr_info("K Reading successfull\n");
	kfree(data);
	return count;
 
}

static int my_write(struct file *file, const char __user *user_buffer,size_t size, loff_t * offset)
{
	pr_info("K Writing to the driver...\n");
    
    if (size <= 0)        return 0;
    pr_info("K Recived bytes:(%d)\n",size);
    
	char *data = (char*)kmalloc(size+1,GFP_USER);
    if (copy_from_user(data, user_buffer, size)) { return -EFAULT;}
	
	pr_info("K RECIVED FROM USER APP:  ");
	pr_info("%s\n",data);
	
    kfree(data);
    
    pr_info("K Writing succesfull\n");
    return size;
}

static long my_ioctl (struct file *file, unsigned int cmd,unsigned long arg)
{
	pr_info("K Reading IOCTL...\n");
	struct my_device_data *my_data = (struct my_device_data*) file->private_data;
	struct my_ioctl_data mid;
	
	switch(cmd) {
		case MY_IOCTL_IN:
			pr_info("K MY_IOCTL_IN...\n");
			if( copy_from_user(&mid, (my_ioctl_data *) arg, sizeof(my_ioctl_data)) )
				return -EFAULT;
     
	 		pr_info("K spinlock to protect the counter\n");
			// Use the spinlock to protect the counter
        	spin_lock(&lock);


			
        	// Reset the counter to the new value
       	 	irqCounter = mid.newCounterVal;


			pr_info("K spin_unlock\n");
        	spin_unlock(&lock);






		break;
			
		default:
			return -ENOTTY;
	}
	
	pr_info("K Reading IOCTL successfull\n");
	
		return 0;
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.release = my_release,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
};


static const struct of_device_id my_driver_dt_ids[] = {

	{ 		.compatible = "uwr,my-driver" 	},
	{	},
};

static int my_driver_probe (struct platform_device *pdev)
{	
	pr_info("K Probing the driver...\n");
	struct resource *res;
	int i, err;
	// register character device driver 
	err = register_chrdev_region(MKDEV(MY_MAJOR, MY_FIRST_MINOR),MY_MAX_MINORS,"my_device_driver");
	/* report error */
	if (err != 0) {	pr_info("K register_chrdev_region error\n"); return err;	}
	pr_info("K Registering succesfull\n");
	for(i = 0; i < MY_MAX_MINORS; i++) {
		/* initialize devs[i] fields */
		cdev_init(&devs[i].cdev, &my_fops);
		cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
	}  

 	struct my_device_data *dev;
    
    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev) {
        return -ENOMEM;
    }
  /*Map peripherial*/
  pr_info("K Mapping peripherial...\n");

  peripheral = devm_kzalloc(&pdev->dev, sizeof(*peripheral), GFP_KERNEL);
  if (!peripheral)
    return -ENOMEM;

   init_completion(&irq_completion);
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  
  pr_info("K Peripherial Resource start addr: 0x%x\n", res->start);

  //peripheral->regs = devm_ioremap_resource(&pdev->dev, res);
  peripheral->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
  if (IS_ERR(peripheral->regs))
  {
	pr_info("K UNABLE TO Map\n");
 	return PTR_ERR(peripheral->regs);
  }
   writel((u32 __force)cpu_to_le32(1), peripheral->regs + INTERRUPT_ENABLE_REG);
	/*Interrupts in the driver*/

	pr_info("K Registering IRQ...\n");

	int irq = platform_get_irq(pdev, 0);

	pr_info("K Got interrupt %d\n", irq);
	
	if (irq < 0) {       
		pr_info("K Failed to get interrupt\n");
		return irq;
	}

	err = devm_request_irq(&pdev->dev, irq, my_irq_handler, IRQF_SHARED, pdev->name, pdev);

	if (err) { pr_info("K Failed to requestinterrupt\n");return err;}
	
	pr_info("K Mapping  succesfull\n");

	pr_info("K Registering IRQ successfull\n");
	return 0;
}

static int my_driver_remove (struct platform_device *pdev)
{
	
	pr_info("K Removing the driver...\n");
	
	int i;
 	for (i = 0; i < MY_MAX_MINORS; i++) {
		/* release devs[i] fields */
		cdev_del(&devs[i].cdev);
	}
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
	
	pr_info("K Removing successfull\n");
	return 0;
}

static struct platform_driver my_driver = {
	.driver = {
		.name = "my_awesome_driver" ,
		.of_match_table = my_driver_dt_ids
	 },
	.probe = my_driver_probe,
	.remove = my_driver_remove,
};

static int __init my_init()
{
	pr_info("K Registering the driver!\n" );
	
	return platform_driver_register(&my_driver);
}

static void __exit my_exit()
{
	pr_info("K Exiting, Goodbye! :)\n");
}

module_init(my_init);

module_exit(my_exit);

MODULE_LICENSE("GPL");
