/*
 * scull.h -- definitions for the char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 * Copyright (C) 2011 Vigith Maurice
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#ifndef _SCULL_H_
#define _SCULL_H_


#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0   /* dynamic major by default */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4    /* scull0 through scull3 */
#endif


/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "scull_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of SCULL_QUANTUM bytes.
 *
 * The array (quantum-set) is SCULL_QSET long.
 */
#ifndef CAPACITY
#define CAPACITY    1000
#endif

/*
 * Representation of scull quantum sets.
 */
struct rbuf {
	char data[CAPACITY];  /* Pointer to first quantum set */         /* the current array size */
	unsigned long size;
	unsigned long rp;
	unsigned long wp;     /* amount of data stored here */
	spinlock_t lock;    
	wait_queue_head_t rqueue;
	wait_queue_head_t wqueue;
	struct cdev cdev;	  /* Char device structure		*/
};

struct scull_snapshot {
    unsigned long size;
    unsigned long rp;
    unsigned long wp;
    char data[CAPACITY];
};
#define SCULL_IOC_PEEK _IOWR('k', 1, char[CAPACITY])
/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */


/*
 * The different configurable parameters
 */
extern int scull_major;     /* main.c */
extern int scull_nr_devs;


/*
 * Prototypes for shared functions
 */

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);                   
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);   


#endif /* _SCULL_H_ */

