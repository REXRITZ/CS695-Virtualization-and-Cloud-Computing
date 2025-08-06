#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include "spock_ioctl.h"


#define MINOR_COUNT 1

// take pid of current running process as cmd argument
// pid_t pid = -1;
// module_param(pid, int, 0);

static struct class *cls = NULL;
static struct cdev cDev;
dev_t dev = 0;

int findPhysicalAddr(struct data* info) {
    
    unsigned long vaddr = info->vaddr;

    pgd_t *pgd;
    p4d_t *p4dt;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    struct task_struct* task = current;

    // walk the page table to get physical address
    pgd = pgd_offset(task->mm, vaddr);
    if(pgd_none(*pgd)) {
        pr_err("VA not mapped: Virtual address is not mapped to page global directory\n");
        return -1;
    }
    p4dt = p4d_offset(pgd, vaddr);
    if(p4d_none(*p4dt)) {
        pr_err("VA not mapped: Virtual address is not mapped to page global directory\n");
        return -1;
    }
    pud = pud_offset(p4dt, vaddr);
    if(pud_none(*pud)) {
        pr_err("VA not mapped: Virtual address is not mapped to page upper directory\n");
        return -1;
    }
    pmd = pmd_offset(pud, vaddr);
    if(pmd_none(*pmd)) {
        pr_err("VA not mapped: Virtual address is not mapped to page middle directory\n");
        return -1;
    }
    pte = pte_offset_kernel(pmd, vaddr);
    if(!pte_present(*pte)) {
        pr_err("VA not mapped: Page table entry not present\n");
        return -1;
    }

    //get page from pte
    struct page* page = pte_page(*pte);
    unsigned long phyAddr = page_to_phys(page);
    phyAddr += (vaddr & ~PAGE_MASK);
    info->phyaddr = phyAddr;
    return 0;
}

// Function to write to physical address
int writeToPhysicalAddress(struct data* info) {

    void __iomem *mem = ioremap(info->phyaddr, sizeof(unsigned char));
    if(!mem)
        return -1;

    printk("PA: 0x%lx val: %d\n", info->phyaddr, ioread8(mem));
    writeb(info->value, mem);
    printk("PA: 0x%lx val: %d\n", info->phyaddr, ioread8(mem));

    // free memory allocated using ioremap
    iounmap(mem);
    return 0;
}

static int spock_open(struct inode* inode, struct file* file) {
    return 0;
}

static int spock_close(struct inode* inode, struct file* file) {
    return 0;
}

static long spock_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    struct data data;
    
    switch(cmd) {
        case V2P:
            if(copy_from_user(&data, (struct data*)arg, sizeof(data)) > 0) {
                printk(KERN_ERR "Getting data from user failed\n");
                return -1;
            }
            if(findPhysicalAddr(&data) < 0) {
                return -1;
            }
            if(copy_to_user((struct data*)arg, &data, sizeof(data)) > 0) {
                return -1;
            }
            break;
        case WRITE_PHY:
            if(copy_from_user(&data, (struct data*)arg, sizeof(data)) > 0) {
                printk(KERN_ERR "Getting data from user failed\n");
                return -1;
            }
            if(writeToPhysicalAddress(&data) < 0) {
                return -1;
            }
            break;

    }
    return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = spock_open,
	.unlocked_ioctl = spock_ioctl,
	.release = spock_close
};

static int initModule(void) {
    
    struct device* devret;

    // register device dynamically
    // dev gets major and minor number
    if(alloc_chrdev_region(&dev, 0, MINOR_COUNT, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate major number");
        return -1;
    }

    // create device class
    if(IS_ERR(cls = class_create(THIS_MODULE, DEVICE_NAME))) {
        
        unregister_chrdev_region(dev, MINOR_COUNT);
        return PTR_ERR(cls);
    }
    // create device 
    if(IS_ERR(devret = device_create(cls, NULL, dev, NULL, DEVICE_NAME))) {
        class_destroy(cls);
        unregister_chrdev_region(dev, MINOR_COUNT);
        return PTR_ERR(devret);
    }

    // initialze and add character device
    cdev_init(&cDev, &fops);
    cDev.owner = THIS_MODULE;
    if(cdev_add(&cDev, dev, MINOR_COUNT) < 0) {
        device_destroy(cls, dev);
        class_destroy(cls);
        unregister_chrdev_region(dev, MINOR_COUNT);
        return -1;
    }

    printk(KERN_INFO "Driver loaded successfully\n");

    return 0;
}

static void cleanupModule(void) {
    device_destroy(cls,dev);
    class_destroy(cls);
    cdev_del(&cDev);
    unregister_chrdev_region(dev, MINOR_COUNT);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("2.1 driver kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");