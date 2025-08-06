#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/mm.h>


#define ENTRY_NAME "get_pgfaults"
#define PERMS 0644
#define PARENT NULL
#define BUF_SIZE 128

static char buffer[BUF_SIZE];

static ssize_t helloProcRead(struct file *file, char __user *buf, size_t size, loff_t *offset) {
    
    unsigned long pageFault[NR_VM_EVENT_ITEMS];
    all_vm_events(pageFault);
    unsigned long totalPageFaults = pageFault[PGMAJFAULT] + pageFault[PGFAULT];
    printk(KERN_INFO "Page faults: %lu\n", totalPageFaults);
    
    int len = snprintf(buffer, sizeof(buffer), "Page faults: %lu\n", totalPageFaults);
    buffer[len] = '\0';
    if(*offset >= len || copy_to_user(buf, buffer + *offset, len - *offset) > 0) {
        printk(KERN_ERR "Copy to user failed\n");
        return 0;
    } else {
        printk("procfile read %s\n", file->f_path.dentry->d_name.name); 
        *offset += len - *offset; 
    }

    return 0;
}

static struct proc_ops fops = {
    .proc_read = helloProcRead
};

static int initModule(void) {
    printk(KERN_INFO "get_pgfault module loaded\n");

    if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
        printk(KERN_ERR "Proc create\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -1;
    }
    return 0;
}
static void cleanupModule(void) {
    remove_proc_entry(ENTRY_NAME, NULL);
    printk(KERN_INFO "get_pgfault module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("get_pgfault procfs kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");