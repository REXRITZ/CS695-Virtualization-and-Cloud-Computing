#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#define HUGE_PAGE_SIZE 1024 * 1024 * 2

// take command line input from here
pid_t pid = -1;
module_param(pid, int, 0);

static int initModule(void) {
    printk(KERN_INFO "LKM5 module loaded\n");
    struct task_struct* process;
    // checking if given pid is valid or not
    for_each_process(process) {
        if(process->pid == pid)
            break;
    }
    if(pid != process->pid) {
        pr_err("Invalid pid entered\n");
        return -1;
    }

    struct mm_struct* mm = process->mm;
    long hugePageCount = 0;
    unsigned long hugePageMemory = 0;

    struct vm_area_struct* vma;
    VMA_ITERATOR(vmi, mm, 0);
    // iterating through process' vma to check if HUGETLB flag is present
    for_each_vma(vmi, vma) {
        for(unsigned long addr = vma->vm_start; addr < vma->vm_end; addr += HPAGE_PMD_SIZE) {
            pgd_t *pgd;
            p4d_t *p4dt;
            pud_t *pud;
            pmd_t *pmd;
            pgd = pgd_offset(vma->vm_mm, addr);
            if(pgd_none(*pgd)) {
                printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page global directory\n");
                continue;
            }
            p4dt = p4d_offset(pgd, addr);
            if(p4d_none(*p4dt)) {
                printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page global directory\n");
                continue;
            }
            pud = pud_offset(p4dt, addr);
            if(pud_none(*pud)) {
                printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page upper directory\n");
                continue;
            }
            pmd = pmd_offset(pud, addr);
            if(pmd_none(*pmd)) {
                printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page middle directory\n");
                continue;
            }
            if(pmd_trans_huge(*pmd)) {
                hugePageCount++;
                hugePageMemory += HPAGE_PMD_SIZE;
                printk(KERN_INFO "Huge found\n");
            }
        }
    }
    hugePageMemory /= 1024;  // converting to KB

    printk(KERN_INFO "THP Size: %lu kB, THP count: %ld\n", hugePageMemory, hugePageCount);

    return 0;
}

static void cleanupModule(void) {
    printk(KERN_INFO "LKM5 module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);


MODULE_DESCRIPTION("lkm5 kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");