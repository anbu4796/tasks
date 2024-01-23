#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include <linux/delay.h>
#include "ioctl_test.h"
#include <linux/mutex.h>

DEFINE_MUTEX(mylock);
#define CHAR_DEV_NAME "chat"
#define MAX_LENGTH length
#define SUCCESS 0
#define FAILURE -1
#define BUF_SIZE 64
static char *buffer[10];
struct cdev *my_cdev;
dev_t mydev;
int count=1;
int buf_counter=-1;
static struct class *my_class;

static int char_dev_open(struct inode *inode,
			    struct file  *file)
{
	static int counter = 0;
	counter++;
	printk(KERN_INFO "Number of times open() was called: %d\n", counter);
	printk (KERN_INFO " MAJOR number = %d, MINOR number = %d\n",imajor (inode), iminor (inode));
	printk(KERN_INFO "Process id of the current process: %d\n",current->pid );
	printk (KERN_INFO "ref=%d\n", module_refcount(THIS_MODULE));
	return SUCCESS;
}

static int char_dev_release(struct inode *inode,
		            struct file *file)
{
	return SUCCESS;
}


static long int my_ioctl(struct file *file, unsigned cmd, unsigned long arg){

	switch(cmd){
		case WR_VALUE:
		
		    
		    buf_counter++;
		    if (buf_counter==10)
		    {
		        printk("Buffer full read first\n");
		        buf_counter--;
		        return FAILURE;
		    }
		    else
		    {
		    buffer[buf_counter] = (char *)kmalloc((BUF_SIZE),GFP_KERNEL);
		    
			if(copy_from_user(buffer[buf_counter], (int32_t *) arg, BUF_SIZE)) 
			{
				printk(" Error copying data from user\n");
			}
			else
			{
				printk("Driver stored the received data into the buffer :  %s \n",buffer[buf_counter]);
			}
			}
			break;
		case RD_VALUE:
		
		    if(buf_counter==-1)
		    {
		        printk("Buffer empty\n");
		        return FAILURE;
		    }
		    else
		    {
		    if(mutex_lock_interruptible(&mylock))
		    {
                  return -ERESTARTSYS;
            }
            else
            {
		    printk(" Process acquired the lock\n");
		    }
            
			if(copy_to_user((int32_t *) arg, buffer[buf_counter], BUF_SIZE)) 
			{

				printk("Error copying data to user\n");
				
			}	
			else
			{
				printk("Driver sent the buffer to user\n");
				mutex_unlock(&mylock);
				kfree(buffer[buf_counter]);
				buf_counter--;
			}
			}
			break;
			
		default:
			printk("Read and write are not executed.....");
			break;
	}
	return 0;
}




static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.open = char_dev_open,
	.unlocked_ioctl = my_ioctl,
	.release = char_dev_release,
};

static __init int char_dev_init(void)
{
	int ret;

	if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) < 0) {
            printk (KERN_ERR "failed to reserve major/minor range\n");
            return -1;
    }

        if (!(my_cdev = cdev_alloc ())) {
            printk (KERN_ERR "cdev_alloc() failed\n");
            unregister_chrdev_region (mydev, count);
            return -1;
 	}
	cdev_init(my_cdev,&char_dev_fops);

	ret=cdev_add(my_cdev,mydev,count);
	if( ret < 0 ) {
		printk(KERN_INFO "Error registering device driver\n");
	        cdev_del (my_cdev);
                unregister_chrdev_region (mydev, count); 	
		return -1;
	}

	my_class = class_create (THIS_MODULE, "Virtualioctl");
        device_create (my_class, NULL, mydev, NULL, "%s", "ioctl");

	printk(KERN_INFO"\nDevice Registered: %s\n",CHAR_DEV_NAME);
	printk (KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR (mydev),MINOR (mydev));

	buffer[0] =(char *)kmalloc((BUF_SIZE),GFP_KERNEL);
	return 0;
}

static __exit void  char_dev_exit(void)
{
	 device_destroy (my_class, mydev);
         class_destroy (my_class);
	 cdev_del(my_cdev);
	 unregister_chrdev_region(mydev,1);
	 kfree(buffer);
	 printk(KERN_INFO "\n Driver unregistered \n");
}
module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_AUTHOR("anburaj");
MODULE_DESCRIPTION("synchronization");
MODULE_LICENSE("GPL");

