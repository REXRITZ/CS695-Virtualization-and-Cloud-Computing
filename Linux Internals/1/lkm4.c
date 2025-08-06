#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <linux/mm.h>
#include <linux/mm_types.h>


// take command line input from here
pid_t pid = -1;
module_param(pid, int, 0);


static int initModule(void) {

    struct task_struct* process;
    // checking if given pid is valid or not
    for_each_process(process) {
        if(process->pid == pid)
            break;
    }
    if(pid != process->pid) {
        pr_err("Invalid pid:%d entered\n", pid);
        return -1;
    }
    struct mm_struct* mm = process->mm;
    struct vm_area_struct* vma;
    
    VMA_ITERATOR(vmi, mm, 0);
    unsigned long totalVaSize = 0;
    // calculate virtual address space from virtual memory regions occupied
    for_each_vma(vmi, vma) {
        totalVaSize += (vma->vm_end - vma->vm_start);
    }

    // get_mm_rss(mm) returns #pages, so multiply by page size to get value in bytes
    // Physical memory = MM_FILE_PAGES + MM_ANONPAGES + MM_SHMEMPAGES
    unsigned long rss = get_mm_rss(mm) * PAGE_SIZE;

    // test correctness using /proc/pid/status -> VmSize and VmRSS fields
    printk(KERN_INFO "[LKM4] Virtual Memory Size: %lu kB", totalVaSize/1024);
    printk(KERN_INFO "[LKM4] Physical Memory Size: %lu kB", rss/1024);

    return 0;
}

static void cleanupModule(void) {
    printk(KERN_INFO "LKM4 module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("lkm4 kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");