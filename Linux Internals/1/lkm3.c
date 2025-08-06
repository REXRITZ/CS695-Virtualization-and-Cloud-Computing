#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <linux/mm.h>

// take command line input from here
pid_t pid = -1;
unsigned long vaddr = 0;
module_param(pid, int, 0);
module_param(vaddr, ulong ,0);

/* Function to find physical address from given virtual address */
int findAddress(unsigned long vaddr, struct mm_struct* mm) {
    /*
        reference to walk page table
        https://www.kernel.org/doc/gorman/html/understand/understand006.html --> wrong??
        new reference
        https://docs.kernel.org/mm/page_tables.html
    */
    pgd_t *pgd;
    p4d_t *p4dt;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    pgd = pgd_offset(mm, vaddr);
    if(pgd_none(*pgd)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page global directory\n");
        return -1;
    }
    p4dt = p4d_offset(pgd, vaddr);
    if(p4d_none(*p4dt)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page global directory\n");
        return -1;
    }
    pud = pud_offset(p4dt, vaddr);
    if(pud_none(*pud)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page upper directory\n");
        return -1;
    }
    pmd = pmd_offset(pud, vaddr);
    if(pmd_none(*pmd)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Virtual address is not mapped to page middle directory\n");
        return -1;
    }
    pte = pte_offset_kernel(pmd, vaddr);
    if(!pte_present(*pte)) {
        printk(KERN_ERR "[LKM3] VA not mapped: Page table entry not present\n");
        return -1;
    }

    //get page from pte
    struct page* page = pte_page(*pte);
    // get physical address of page
    unsigned long phyAddr = page_to_phys(page);
    // add offset from vaddr
    phyAddr += (vaddr & ~PAGE_MASK);
    printk(KERN_INFO "[LKM3] Virtual address: 0x%lx\n Physical address: 0x%lx\n", vaddr, phyAddr);
    return 0;
}

static int initModule(void) {

    struct task_struct* process;
    // checking if given pid is valid or not
    for_each_process(process) {
        if(process->pid == pid)
            break;
    }
    if(pid != process->pid) {
        printk(KERN_ERR "Invalid pid entered\n");
        return -1;
    }

    struct mm_struct* mm = process->mm;

    // unsigned long va = 0x55633c78a028;
    return findAddress(vaddr, mm);

}

static void cleanupModule(void) {
    printk(KERN_INFO "LKM3 module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("lkm3 kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");