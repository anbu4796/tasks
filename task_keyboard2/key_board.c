#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>
#define CHAR_DEV_NAME "task"
#define SUCCESS 0
#define SIGETX 44
#define REG_CURRENT_TASK _IOW('a','a',int32_t*)
 
static struct task_struct *task = NULL;
static int signum = 0;
struct cdev *my_cdev;
dev_t mydev;
int count =1;
static struct class *my_class;
const unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', 'E',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    'C',	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

MODULE_LICENSE("GPL");
static int irq = 1,  dev = 0xaa;
#define KBD_DATA_REG        0x60    /* I/O port for keyboard data */
#define KBD_SCANCODE_MASK   0x7f
#define KBD_STATUS_MASK     0x80

static irqreturn_t keyboard_handler(int irq, void *dev)
{
	char scancode;
	scancode = inb(KBD_DATA_REG);
	pr_info("Character %c %s\n",
			kbdus[scancode & KBD_SCANCODE_MASK],
			scancode & KBD_STATUS_MASK ? "Released" : "Pressed");
			struct kernel_siginfo info;
    pr_info("Shared IRQ: Interrupt Occurred");
    
    //Sending signal to app
    memset(&info, 0, sizeof(struct kernel_siginfo));
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = kbdus[scancode & KBD_SCANCODE_MASK];
 
    if (task != NULL) {
        pr_info("Sending signal to app\n");
        if(send_sig_info(SIGETX, &info, task) < 0) {
            pr_info("Unable to send signal\n");
        }
    }
 
        return IRQ_NONE;
}



static int key_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
 
static int key_release(struct inode *inode, struct file *file)
{
    struct task_struct *ref_task = get_current();
    pr_info("Device File Closed...!!!\n");
    
    //delete the task
    if(ref_task == task) {
        task = NULL;
    }
    return 0;
}
 
static ssize_t key_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
   // asm("int $0x3B");  
    return 0;
}

static ssize_t key_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}
 
static long key_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == REG_CURRENT_TASK) {
        pr_info("REG_CURRENT_TASK\n");
        task = get_current();
        signum = SIGETX;
    }
    return 0;
}
 
 static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = key_read,
        .write          = key_write,
        .open           = key_open,
        .unlocked_ioctl = key_ioctl,
        .release        = key_release,
};
/* registering irq */
static int test_interrupt_init(void)
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
	cdev_init(my_cdev,&fops);

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

        pr_info("%s: In init\n", __func__);
        return request_irq(irq, keyboard_handler, IRQF_SHARED,"my_keyboard_handler", &dev);
}

static void test_interrupt_exit(void)
{
        device_destroy (my_class, mydev);
        class_destroy (my_class);
	    cdev_del(my_cdev);
	    unregister_chrdev_region(mydev,1);
	    printk(KERN_INFO "\n Driver unregistered \n");
        pr_info("%s: In exit\n", __func__);
        synchronize_irq(irq); /* synchronize interrupt */
        free_irq(irq, &dev);
}

module_init(test_interrupt_init);
module_exit(test_interrupt_exit);
