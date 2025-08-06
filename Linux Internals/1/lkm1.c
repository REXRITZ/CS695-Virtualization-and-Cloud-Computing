#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>


struct task_struct* task;

static int initModule(void) {

    printk(KERN_INFO "[LKM1] Runnable processes:\n");
    printk(KERN_INFO "[LKM1] %10s %10s\n", "PID", "PROC");

    for_each_process(task) {
        /* Check if process is in Running/Runnable state */
        if(task->__state == TASK_RUNNING) {
            printk(KERN_INFO "[LKM1] %10d %10s\n", task->pid, task->comm);
        }
    }
    return 0;
}

static void cleanupModule(void) {
    printk(KERN_INFO "Module LKM1 unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("LKM1 kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");