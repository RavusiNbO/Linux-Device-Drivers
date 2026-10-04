


 #include <linux/module.h>
 #include <linux/moduleparam.h>
 #include <linux/init.h>
 
 #include <linux/kernel.h>
 #include <linux/slab.h>
 #include <linux/fs.h>
 #include <linux/errno.h>
 #include <linux/types.h>
 #include <linux/string.h>
 #include <linux/fcntl.h>
 #include <linux/seq_file.h>
 #include <linux/cdev.h>
 #include <linux/uaccess.h>
 
 #include "scull.h"	/* local definitions */

/*
 * Our parameters which can be set at load time.
 */

int scull_major =   SCULL_MAJOR;
int scull_minor =   0;
int scull_nr_devs = SCULL_NR_DEVS;
static char end = 0;

MODULE_AUTHOR("Fakhretdinov Ravil");
MODULE_LICENSE("GPL");

struct rbuf *scull_devices;	/* allocated in scull_init_module */


/*
 * Empty out the scull device; must be called with the device
 * semaphore held.
 */

/*
 * Open and close
 */

int scull_open(struct inode *inode, struct file *filp)
{
	struct rbuf *dev; /* device information */

	dev = container_of(inode->i_cdev, struct rbuf, cdev);
	filp->private_data = dev; /* for other methods */
	return 0;          /* success */
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}
/*
 * Data management: read and write
 */

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	if (!count) return 0;
	struct rbuf *dev = filp->private_data;
	int retvalue = 0;
	char *kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf){
		retvalue = -ENOMEM;
		goto read_out;
	} 
	unsigned readed = 0, to_read = 0;
	


	while (readed != count)
	{
	wait_read:
		wait_event_interruptible(dev->rqueue, dev->size != 0 || end);
		if (end){
			retvalue = -ENODEV;
			goto read_out;
		}
		spin_lock(&dev->lock);
		if (dev->size == 0) {
			spin_unlock(&dev->lock); 
			goto wait_read;
		}
		if (dev->wp <= dev->rp)
		{
			to_read = CAPACITY - dev->rp < count - readed ? CAPACITY - dev->rp : count - readed;
			memcpy(kbuf + readed, dev->data + dev->rp, to_read);
			readed += to_read;
			dev->size -= to_read;
			dev->rp = (dev->rp + to_read) % CAPACITY;
		}
		if (readed < count) 
		{
			to_read = dev->wp - dev->rp < count - readed ? dev->wp - dev->rp : count - readed;
			memcpy(kbuf + readed, dev->data, to_read);
			readed += to_read;
			dev->size -= to_read;
			dev->rp += to_read;
		}
		spin_unlock(&dev->lock);
		if (end) {
			retvalue = -ENODEV;
			goto read_out;
		}
		if (dev->size == 1) wake_up_interruptible(&dev->wqueue);
	}
	if (copy_to_user(buf, kbuf, count)) retvalue = -EFAULT;

read_out:
	if (kbuf) kfree(kbuf);
	return retvalue;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	if (!count) return 0;
	struct rbuf *dev = filp->private_data;
	int retvalue = 0;
	char *kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf) {
		retvalue = -ENOMEM; 
		goto write_out;
	}
	unsigned writed = 0, to_write = 0;
	if (copy_from_user(kbuf, buf, count)) 
	{
		retvalue = -EFAULT; 
		goto write_out;
	}
	while (writed != count)
	{
	wait_write:
		wait_event_interruptible(dev->wqueue, dev->size != CAPACITY || end);
		if (end){
			retvalue = -ENODEV;
			goto write_out;
		}
		spin_lock(&dev->lock);
		if (dev->size == CAPACITY) {
			spin_unlock(&dev->lock); 
			goto wait_write;
		}
		if (dev->wp >= dev->rp)
		{
			to_write = CAPACITY - dev->wp < count - writed ? CAPACITY - dev->wp : count - writed;
			memcpy(dev->data + dev->wp, kbuf + writed, to_write);
			writed += to_write;
			dev->wp = (dev->wp + to_write) % CAPACITY;
		}
		if (writed < count) 
		{
			to_write = dev->rp - dev->wp < count - writed ? dev->rp - dev->wp : count - writed;
			memcpy(kbuf + writed, dev->data, to_write);
			writed += to_write;
			dev->wp += to_write;
		}
		spin_unlock(&dev->lock);
		if (end){
			retvalue = -ENODEV;
			goto write_out;
		}
		if (dev->size == CAPACITY - 1) wake_up_interruptible(&dev->rqueue);
	}

write_out:
	if (kbuf) kfree(kbuf);
	return retvalue;
}


/*
 * The "extended" operations -- only seek
 */




struct file_operations scull_fops = {
	.owner =    THIS_MODULE,
	.read =     scull_read,
	.write =    scull_write,
	.open =     scull_open,
	.release =  scull_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void scull_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(scull_major, scull_minor);
	end = 1;

	/* Get rid of our char dev entries */
	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			wake_up_all(&scull_devices[i].rqueue);
			wake_up_all(&scull_devices[i].wqueue);
			kfree(scull_devices[i].data);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, scull_nr_devs);

}


/*
 * Set up the char_dev structure for this device.
 */
static void scull_setup_cdev(struct rbuf *dev, int index)
{
	int err, devno = MKDEV(scull_major, scull_minor + index);
    
	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}


int scull_init_module(void)
{
	int result, i;
	dev_t dev = 0;

/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
				"scull");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	scull_devices = kmalloc(scull_nr_devs * sizeof(struct rbuf), GFP_KERNEL);
	if (!scull_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(scull_devices, 0, scull_nr_devs * sizeof(struct rbuf));

        /* Initialize each device. */
	for (i = 0; i < scull_nr_devs; i++) {
		scull_devices[i].wp = 0;
		scull_devices[i].rp = 0;
		scull_devices[i].size = 0;
		spin_lock_init(&scull_devices[i].lock);
		init_waitqueue_head(&scull_devices->rqueue);
		init_waitqueue_head(&scull_devices->wqueue);
		scull_setup_cdev(&scull_devices[i], i);
	}

	return 0; /* succeed */

  fail:
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);

