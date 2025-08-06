#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/delay.h>

#include "bloom_ioctl.h" 


#define MINOR_COUNT 1

static struct class *cls = NULL;
static struct cdev cDev;
dev_t dev = 0;

static int bloom_open(struct inode* inode, struct file* file) {
    return 0;
}

static int bloom_close(struct inode* inode, struct file* file) {
    return 0;
}

struct task_struct* getTask(pid_t pid) {
    struct task_struct *task;
    for_each_process(task) {
        if(task->pid == pid) {
            return task;
        }
    }
    return NULL;
}

// Function to change parent of a process
int makeParent(pid_t pid) {

    struct task_struct *newParent = getTask(pid);
    struct task_struct *currentProcess = current;

    if(newParent == NULL|| currentProcess == NULL) {
        pr_err("MAKE_PARENT: Invalid pid entered\n");
        return -1;
    }

    // copy current process parent in tmp variable
    struct task_struct *orignalParent = currentProcess->parent;
    struct list_head *childNode;

    task_lock(orignalParent);
    // iterate parent's children list of current process to remove current process node from list
    list_for_each(childNode, &orignalParent->children) {
        struct task_struct *child = list_entry(childNode, struct task_struct, sibling);
        if(child->pid == currentProcess->pid) {
            list_del(childNode);
            break;
        }
    }
    task_unlock(orignalParent);

    task_lock(currentProcess);
    task_lock(newParent);

    // set new parent 
    currentProcess->real_parent = newParent;
    currentProcess->parent = newParent;

    // add current process to children list of new parent
    list_add_tail(childNode, &newParent->children);

    // wake up parent process if it is sleeping
    wake_up_process(newParent);

    task_unlock(newParent);
    task_unlock(currentProcess);

    return 0;


}

// Function to kill all child processes of a process with given pid
int killAllChildProcess(pid_t pid) {

    struct task_struct *parent = getTask(pid);
    
    if(parent == NULL) {
        pr_err("KILL_CHILD: Invalid pid entered\n");
        return -1;
    }
    struct task_struct* child;
    // task_lock(parent);
    list_for_each_entry(child, &parent->children, sibling) {
        printk(KERN_INFO "child process pid: %d\n", child->pid);
        task_lock(child);
        if(send_sig(SIGTERM, child, 1) < 0) {
            pr_err("Unable to call SIGTERM on child process PID: %d\n", child->pid);
            return -1;
        }
        task_unlock(child);
        ssleep(1);

    }
    // task_unlock(parent);
    if(send_sig(SIGTERM, parent, 1) < 0) {
        pr_err("Unable to kill process PID: %d\n", parent->pid);
        return -1;
    }

    return 0;
}

static long bloom_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    
    pid_t pid;
    int ret = 0;
    switch(cmd) {
        case MODIFY_PARENT:
            if(copy_from_user(&pid, (pid_t*)arg, sizeof(pid_t)) > 0) {
                pr_err("Getting data from user failed\n");
                ret = -1;
            }
            if(ret != -1)
                if(makeParent(pid) < 0)
                    ret = -1;
            break;
        case KILL_CHILD:
            if(copy_from_user(&pid, (pid_t*)arg, sizeof(pid_t)) > 0) {
                pr_err("Getting data from user failed\n");
                ret = -1;
            }
            if(ret != -1)
                if(killAllChildProcess(pid) < 0)
                    ret = -1;
            break;
        default:
            pr_err("Invalid command entered\n");
            ret = -1;
            break;

    }
    return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = bloom_open,
	.unlocked_ioctl = bloom_ioctl,
	.release = bloom_close
};

static int initModule(void) {
    
    struct device* devret;

    // register device dynamically
    // dev gets major and minor number
    if(alloc_chrdev_region(&dev, 0, MINOR_COUNT, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate major number");
        return -1;
    }


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

MODULE_DESCRIPTION("bloom driver kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");