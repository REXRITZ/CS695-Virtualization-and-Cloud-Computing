#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>
#include <linux/list.h>

// take command line input from here
pid_t pid = -1;
module_param(pid, int, 0);

/* 
    converts state of type unsigned int to char*  
*/
char* getState(unsigned int state) {
    switch (state)
    {
    case TASK_RUNNING:
        return "TASK_RUNNING";
    case TASK_STOPPED:
        return "TASK_STOPPED";
    case TASK_INTERRUPTIBLE:
        return "TASK_INTERRUPTIBLE";
    case TASK_UNINTERRUPTIBLE:
        return "TASK_UNINTERRUPTIBLE";
    case TASK_DEAD:
        return "TASK_DEAD";
    }
    return "TASK UNKNOWN";
}

static int initModule(void) {

    struct task_struct* parent;
    // checking if given pid is valid or not
    for_each_process(parent) {
        if(parent->pid == pid)
            break;
    }

    if(pid != parent->pid) {
        pr_err("Invalid pid entered\n");
        return -1;
    }

    struct list_head* childlist;
    struct task_struct* child;

    // macro present in linux/list.h
    // traversing parent processes child present in task_struct -> list_head children data structure
    list_for_each(childlist, &parent->children) {
        child = list_entry(childlist, struct task_struct, sibling);
        printk(KERN_INFO "[LKM2] Child process PID: %d, State: %s\n", child->pid, getState(child->__state));
    }
    return 0;
}

static void cleanupModule(void) {
    printk(KERN_INFO "LKM2 module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("LKM2 kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");