#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mm.h>

#define BUF_SIZE 512
#define PID_FILE "pid"
#define VIRT_MEM_FILE "virtmem"
#define PHY_MEM_FILE "phymem"
#define UNIT_FILE "unit"

struct kobject kobj;
static int pid = -1;
static long virtmem = -1;
static long phymem = -1;
static char unit = 'B';

static ssize_t fileShow(struct kobject *kobject, struct attribute *attr, char *buf) {
    int ret = 0;

    // attr->name -- name of attribute where read is done
    if(strcmp(attr->name, PID_FILE) == 0)
        ret = sysfs_emit(buf, "%d\n", pid);
    else if(strcmp(attr->name, VIRT_MEM_FILE) == 0)
        ret = sysfs_emit(buf, "%ld\n", virtmem);
    else if(strcmp(attr->name, PHY_MEM_FILE) == 0)
        ret = sysfs_emit(buf, "%ld\n", phymem);
    else if(strcmp(attr->name, UNIT_FILE) == 0)
        ret = sysfs_emit(buf, "%c\n", unit);
    else {
        pr_err("Attribute %s not found\n", attr->name);
        ret = -EINVAL;
    }
    return ret;
}

// taken from 1.4 problem
void updateStats(int tmpPid) {
    struct task_struct* process;

    for_each_process(process) {
        if(process->pid == tmpPid)
            break;
    }
    if(tmpPid != process->pid) {
        pr_err("Invalid pid entered\n");
        return;
    }
    struct mm_struct* mm = process->mm;
    struct vm_area_struct* vma;
    
    VMA_ITERATOR(vmi, mm, 0);
    long totalVaSize = 0;
    for_each_vma(vmi, vma) {
        totalVaSize += (vma->vm_end - vma->vm_start);
    }
    long rss = get_mm_rss(mm) << PAGE_SHIFT;
    virtmem = totalVaSize;
    phymem = rss;
    pid = tmpPid;
}

// function to change size format and update virtmem and phymem accordingly
void updateSizeFormat(char oldUnit, char newUnit) {
    if(newUnit != 'B' && newUnit != 'M' && newUnit != 'K') {
        pr_err("Invalid unit entered\n");
        return;
    }
    if(virtmem == -1 || phymem == -1) {
        pr_err("Some error\n");
        return;
    }

    if(oldUnit == 'K') {
        virtmem *= 1024;
        phymem *= 1024;
    } else if(oldUnit == 'M') {
        virtmem *= 1024 * 1024;
        phymem *= 1024 * 1024;
    }
    if(newUnit == 'M') {
        virtmem /= (1024 * 1024);
        phymem /= (1024 * 1024);
    } else if(newUnit == 'K') {
        virtmem /= 1024;
        phymem /= 1024;
    }
    unit = newUnit;
}

static ssize_t fileStore(struct kobject *kobject, struct attribute *attr, const char *buf, size_t count) {

    if(strcmp(attr->name, PID_FILE) == 0) {
        int tmpPid = -1;
        sscanf(buf, "%d", &tmpPid);
        updateStats(tmpPid);
    }
    else if(strcmp(attr->name, UNIT_FILE) == 0) {
        char tmpUnit;
        sscanf(buf, "%c", &tmpUnit);
        updateSizeFormat(unit, tmpUnit);
    }
    else {
        pr_err("Attribute %s not valid\n", attr->name);
    }
  
    return count;
}

static void fileRelease(struct kobject *kobject)
{
    // do something here later??
}

struct sysfs_ops sops = {
    .show = fileShow,
    .store = fileStore,
};

static struct attribute pidFileAttribute = {
    .name = PID_FILE,
    .mode = 0664,
};

static struct attribute virtmemFileAttribute = {
    .name = VIRT_MEM_FILE,
    .mode = 0444,
};

static struct attribute phymemFileAttribute = {
    .name = PHY_MEM_FILE,
    .mode = 0444,
};

static struct attribute unitFileAttribute = {
    .name = UNIT_FILE,
    .mode = 0664,
};

static struct attribute *file_attrs[] = {
    &pidFileAttribute,
    &virtmemFileAttribute,
    &phymemFileAttribute,
    &unitFileAttribute,
    NULL,
};

// static const struct attribute_group file_group = {
// 	.attrs = file_attrs,
// };
ATTRIBUTE_GROUPS(file);

static const struct kobj_type myKtype = {
    .sysfs_ops = &sops,
    .release = fileRelease,
    .default_groups = file_groups,
};

static int initModule(void) {
    printk(KERN_INFO "get_memstats module loaded\n");

    int ret = kobject_init_and_add(&kobj, &myKtype, kernel_kobj, "%s", "mem_stats");
    if(ret)
        return -1;
    return 0;
}
static void cleanupModule(void) {
    kobject_put(&kobj);
    printk(KERN_INFO "get_memstats module unloaded\n");
}

module_init(initModule);
module_exit(cleanupModule);

MODULE_DESCRIPTION("Helloworld get_memstats kernel module");
MODULE_AUTHOR("Ritesh");
MODULE_LICENSE("GPL");