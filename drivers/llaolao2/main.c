/*
 Example of a minimal character device driver 
 Usage examples:	
 kernel stack overflow:
 echo 2000 > /sys/kernel/debug/llaolao/age
 
 Porbe
 echo probe > /proc/llaolao
  	
*/
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/dcache.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include "gearm.h"

#define DEVICE_NAME      "huadeng2"
#define MAJOR_NUM        88
#define NUM_DEVICES      6
#define DEV_DATA_LENGTH  100
#define TARGET_NAME_MAX  16

static struct class *huadeng_class;
struct huadeng_dev {
  unsigned short current_pointer; /* Current pointer within the
                                     device data region */
  unsigned int size;              /* Size of the bank */
  int number;                     /* device number */
  struct cdev cdev;               /* The cdev structure */
  char name[10];                  /* Name of the device */
  char data[DEV_DATA_LENGTH];
  /* ... */                       /* Mutexes, spinlocks, wait
                                     queues, .. */
} *hd_devp[NUM_DEVICES];

struct lll_profile_struct {
    int             age;
    struct file *   file;
    // other stuffs
};
static char* hostip = "192.168.0.101";
static char* targetip = "192.168.8.1";
static char targetname[TARGET_NAME_MAX];
static unsigned short hostport = 5000;
static unsigned short targetport = 6000;
static struct proc_dir_entry *proc_lll_entry = NULL ;
int g_seed = 0;

// static varibales for debugfs
static struct dentry *df_dir = NULL, * df_dir;

static void wastestack(int recursive)
{
	printk("wastestack %d\n",recursive);

	while(recursive--)
		wastestack(recursive);
}

long ge_probe_kernel_read(void *dst, const void *src, size_t size)
{
	long ret;
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	pagefault_disable();
	ret = __copy_from_user_inatomic(dst,
			(__force const void __user *)src, size);
	pagefault_enable();
	set_fs(old_fs);

	return ret ? -EFAULT : 0;
}

static void	ge_probe(void)
{
	char data[32] = {0};
	long addr = 0x880;

    long ret = ge_probe_kernel_read(data, (const void *)addr, sizeof(data));
	printk("probe %lx got %ld\n", addr, ret);
}

static int
lll_age_set(void *data, u64 val)
{
    struct lll_profile_struct * lp = (struct lll_profile_struct *)data;
    lp->age = val;
	if(val==200)
		wastestack(val);

    return 0;
}
static int
lll_age_get(void *data, u64 *val)
{
    struct lll_profile_struct * lp = (struct lll_profile_struct *)data;
    *val = lp->age;

    return 0;
}

// macro from linux/fs.h
DEFINE_SIMPLE_ATTRIBUTE(df_age_fops, lll_age_get, lll_age_set, "%llu\n");

static ssize_t proc_lll_read(struct file *filp, char __user * buf, size_t count, loff_t * offp)
{
    int n = 0, ret;
    char secrets[100];

    printk(KERN_INFO "proc_lll_read is called file %p, buf %p count %ld off %llx\n",
        filp, buf, count, *offp);
    sprintf(secrets, "kernel secrets balabala %s...\n", filp->f_path.dentry->d_iname);	

    n = strlen(secrets);
    if(*offp < n)
    {
	ret = copy_to_user(buf, secrets, n+1);
        *offp = n+1;
        ret = n+1;
    }
    else
        ret = 0;

    return ret; 
}
/* A timer list */
struct timer_list kt_timer;
 
/* Timer callback */
void ll_timer_callback(unsigned long arg)
{
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
 
	*(int*)arg = 0xbad0bad0;
    //mod_timer(&kt_timer, jiffies + 10*HZ); /* restarting timer */
}

int ll_writenull_in_timer(void)
{
/*	init_timer(&kt_timer);
	kt_timer.function = ll_timer_callback;
	kt_timer.data = (unsigned long)0xbad;
	kt_timer.expires = jiffies + 5*HZ; // 5 second 

	add_timer(&kt_timer); // Starting the timer 
    */
	printk(KERN_INFO "readnull_timer is started\n");
	return 0;
}

static void ll_print_pgtable_macro(void)
{
    printk("PAGE_OFFSET = 0x%lx\n", PAGE_OFFSET);
    printk("PGDIR_SHIFT = %d\n", PGDIR_SHIFT);
    printk("PUD_SHIFT = %d\n", PUD_SHIFT);
    printk("PMD_SHIFT = %d\n", PMD_SHIFT);
    printk("PAGE_SHIFT = %d\n", PAGE_SHIFT);

    printk("PTRS_PER_PGD = %d\n", PTRS_PER_PGD);
    printk("PTRS_PER_PUD = %d\n", PTRS_PER_PUD);
    printk("PTRS_PER_PMD = %d\n", PTRS_PER_PMD);
    printk("PTRS_PER_PTE = %d\n", PTRS_PER_PTE);

    printk("PAGE_MASK = 0x%lx\n", PAGE_MASK);
}

static unsigned long ll_v2p(unsigned long vaddr)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
        unsigned long page_addr = 0;
    unsigned long page_offset = 0;

    pgd = pgd_offset(current->mm, vaddr);
    printk("pgd_val = 0x%llx\n", pgd_val(*pgd));
    printk("pgd_index = %lu\n", pgd_index(vaddr));
    if (pgd_none(*pgd)) {
        printk("not mapped in pgd\n");
        return -1;
    }

    pud = pud_offset(pgd, vaddr);
    printk("pud_val = 0x%llx\n", pud_val(*pud));
    if (pud_none(*pud)) {
        printk("not mapped in pud\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr);
    printk("pmd_val = 0x%llx\n", pmd_val(*pmd));
    printk("pmd_index = %lu\n", pmd_index(vaddr));
    if (pmd_none(*pmd)) {
        printk("not mapped in pmd\n");
        return -1;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    printk("pte_val = 0x%llx\n", pte_val(*pte));
    printk("pte_index = %lu\n", pte_index(vaddr));
    if (pte_none(*pte)) {
        printk("not mapped in pte\n");
        return -1;
    }

    /* Page frame physical address mechanism | offset */
    page_addr = pte_val(*pte) & PAGE_MASK;
    page_offset = vaddr & ~PAGE_MASK;
    paddr = page_addr | page_offset;
    printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
        printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

    return paddr;
}

static DEFINE_PER_CPU(unsigned long, ge_counter);
void ge_percpu(void)
{
    unsigned long p0, p1,p2,p3;
    unsigned long old = __this_cpu_read(ge_counter);
    __this_cpu_write(ge_counter, old+1);
    printk("per_cpu_start %lx, per_cpu_end %lx current %lx\n",
        (unsigned long)__per_cpu_start, (unsigned long)__per_cpu_end, (unsigned long)current);

    p0 = (unsigned long)per_cpu_ptr(&ge_counter, 0);
    p1 = (unsigned long)per_cpu_ptr(&ge_counter, 1);
    p2 = (unsigned long)per_cpu_ptr(&ge_counter, 2);
    p3 = (unsigned long)per_cpu_ptr(&ge_counter, 3);

    printk("percpu var at %lx, %lx, %lx %lx\n", p0, p1, p2, p3);
    p0 = (unsigned long)per_cpu_ptr(current, 0);
    p1 = (unsigned long)per_cpu_ptr(current, 1);
    p2 = (unsigned long)per_cpu_ptr(current, 2);
    p3 = (unsigned long)per_cpu_ptr(current, 3);

    printk("percpu current at %lx, %lx, %lx %lx\n", p0, p1, p2, p3);
    //p3 = (unsigned long)per_cpu_ptr(&current, 3);
}
static ssize_t proc_lll_write(struct file *file, const char __user *buffer,
			 size_t count, loff_t *data)
{
    char cmd[100]={0x00}, *para;
    unsigned long addr = 0, para_long = 0;

    printk("proc_lll_write called legnth 0x%lx, %p\n", count, buffer);
    if (count < 1)
    {
       printk("count <=1\n");
       return -EBADMSG; /* runt */
    }
    if (count > sizeof(cmd))
    {
        printk("count > sizeof(cmd)\n");
        return -EFBIG;   /* too long */
    }

    if (copy_from_user(cmd, buffer, count))
        return -EFAULT;
    para = strchr(cmd, ' ');
    if (para) {
        *para = 0;
        para++;
        para_long = simple_strtoul(para, NULL, 0);
    }
    if(strncmp(cmd,"div0",4) == 0)
    {
	printk("going to divide %d/%ld\n", g_seed, count-5);

        g_seed = g_seed/(count-5); 
    }
    else if(strncmp(cmd, "nullp", 5) == 0)
    {
        *(int*)(long)0x880 = 0x88888888;
    }
    else if(strncmp(cmd, "probe", 5) == 0)
    {
		ge_probe();
    }
    else if(strncmp(cmd,"timer0", 6) == 0)
    {
	ll_writenull_in_timer();
    }
    else if(strncmp(cmd, "pte", 3) == 0)
    {
        addr = para_long;
        addr = (addr == 0) ? ((unsigned long)buffer) : addr;
	    printk("physical address of %lx = %lx\n", addr, ll_v2p(addr));
        ll_print_pgtable_macro();
    }
    else if(strncmp(cmd, "sysreg", 6) == 0)
    {
	    ge_arm_sysregs();
    }
    else if (strncmp(cmd, "jtag", 4) == 0)
    {
        ge_arm_switch_jtag(1);
        ge_arm_enable_jtag_clk(1);
    }
    else if (strncmp(cmd, "jtagoff", 7) == 0)
    {
        ge_arm_switch_jtag(0);
        ge_arm_enable_jtag_clk(0);
    }
    else if (strncmp(cmd, "hot", 3) == 0)
    {
        ge_arm_read_tsadc(0);
        ge_arm_read_tsadc(1);
    }
    else if (strncmp(cmd, "percpu", 6) == 0) {
        ge_percpu();
    }
    else if (strncmp(cmd, "iram", 4) == 0) {
        ge_iram((int)para_long);
    }
    else
    {
        printk("unsupported cmd '%s'\n", cmd);
    }

    return count;	
}

static const struct file_operations proc_lll_fops = {
 .owner = THIS_MODULE,
 .read  = proc_lll_read,
 .write = proc_lll_write,
};

static int huadeng_open(struct inode *inode, struct file *file)
{
  struct huadeng_dev *hd_devp;
  pr_info("%s\n", __func__);

  /* Get the per-device structure that contains this cdev */
  hd_devp = container_of(inode->i_cdev, struct huadeng_dev, cdev);

  /* Easy access to cmos_devp from rest of the entry points */
  file->private_data = hd_devp;

  /* Initialize some fields */
  hd_devp->size = DEV_DATA_LENGTH;
  hd_devp->current_pointer = 0;

  return 0;
}

static int huadeng_release(struct inode *inode, struct file *file)
{
  struct huadeng_dev *hd_devp = file->private_data;
  pr_info("%s\n", __func__);

  /* Reset file pointer */
  hd_devp->current_pointer = 0;

  return 0;
}
int g_vfile_pecent = -1;

static ssize_t huadeng_read(struct file *file,
  char *buffer, size_t count, loff_t * offset)
{
  struct huadeng_dev *hd_devp = file->private_data;
  pr_info("%s %lu, +%llx\n", __func__, count, *offset);

  if (*offset >= hd_devp->size) {
    return 0; /*EOF*/
  }
  /* Adjust count if its edges past the end of the data region */
  if (*offset + count > hd_devp->size) {
    count = hd_devp->size - *offset;
  }
  /* Copy the read bits to the user buffer */
  if (copy_to_user(buffer, (void *)(hd_devp->data + *offset), count) != 0) {
    return -EIO;
  }
    
  return count;
}

static ssize_t huadeng_write(struct file *file,
  const char *buffer, size_t length, loff_t * offset)
{
	int i;
	unsigned long percent;

	struct huadeng_dev *hd_devp = file->private_data;
	pr_info("%s %lu +%llx\n", __func__, length, *offset);
	if(*offset >= hd_devp->size) {
		return 0;
	}  
	if (*offset + length > hd_devp->size) {
		length = hd_devp->size - *offset;
	}
	if(copy_from_user(hd_devp->data+*offset, buffer, length) !=0 ) {
		printk(KERN_ERR "copy_from_user failed\n");
		return -EFAULT;
	} 
    strcpy(targetname, buffer);
	percent = simple_strtoul(buffer, NULL, 0x10);

	printk("v_store: %s %ld\n", buffer, length);
	for (i = 0; i < length; i++)
		printk(" %c ", buffer[i]);
	printk("\n");

	if (percent > 100)
		g_vfile_pecent = 100;
	else
		g_vfile_pecent = percent;

	//
	//	update the data buffer in dev's private data structure to ease geduers
	sprintf(hd_devp->data, "%d\%%\n", g_vfile_pecent);
	//
 
	return length;
}

struct file_operations huadeng_fops = {
  .owner = THIS_MODULE,
  .open = huadeng_open,
  .release = huadeng_release,
  .read = huadeng_read,
  .write = huadeng_write,
};

static int __init llaolao_init(void)
{
    int n = 0x1937, ret = 0, i;
    static struct lll_profile_struct lll_profile;

    printk(KERN_INFO "Hi, I am llaolao at address: %p\n symbol: 0x%pF\n stack: 0x%p\n"
        " first 16 bytes: %*phC\n",
	    llaolao_init, llaolao_init, &n, 16, (char*)llaolao_init);
#ifdef CHRDRV_OLD_STYLE
    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &huadeng_fops);
    if (ret != 0) {
        printk(KERN_ERR "register_chrdev failed %d\n", ret);
        return ret;
    }
#endif

    huadeng_class = class_create(THIS_MODULE, DEVICE_NAME);
    for (i = 0; i < NUM_DEVICES; i++) {
        /* Allocate memory for the per-device structure */
        hd_devp[i] = kmalloc(sizeof(struct huadeng_dev), GFP_KERNEL);
        if (!hd_devp[i]) {
            printk("allocate huadeng dev struct failed\n"); 
            return -ENOMEM;
        }
        /* Connect the file operations with the cdev */
        cdev_init(&hd_devp[i]->cdev, &huadeng_fops);
        hd_devp[i]->cdev.owner = THIS_MODULE;

        /* Connect the major/minor number to the cdev */
        ret = cdev_add(&hd_devp[i]->cdev, MKDEV(MAJOR_NUM, i), 1);
        if (ret) {
          printk(KERN_ERR "cdev_add failed for %d\n", i);
          return ret;
        }

        /* Send uevents to udev, so it'll create /dev nodes */
        device_create(huadeng_class, NULL,
            MKDEV(MAJOR_NUM, i), NULL, "huadeng%d", i);
    }

    /* Create /proc/llaolao */
    proc_lll_entry = proc_create("llaolao", 0, NULL, &proc_lll_fops);
    if (proc_lll_entry == NULL) {
        printk(KERN_ERR "create proc entry failed\n");
        return -1;
    }
    df_dir = debugfs_create_dir("llaolao", 0);
    if(!df_dir)
    {
        printk(KERN_ERR "create dir under debugfs failed\n");
        return -1;
    }

    debugfs_create_file("age", 0222, df_dir, &lll_profile, &df_age_fops);

    return 0;
}

static void __exit llaolao_exit(void)
{
    int i;
    printk("Exiting from 0x%p... Bye, GEDU friends\n", llaolao_exit);
    if(proc_lll_entry)
        proc_remove(proc_lll_entry);
    if(df_dir)
      // clean up all debugfs entries
      debugfs_remove_recursive(df_dir);
    if(huadeng_class) {
        for (i = 0; i < NUM_DEVICES; i++) {
            device_destroy(huadeng_class, MKDEV(MAJOR_NUM, i));
            cdev_del(&hd_devp[i]->cdev);
            kfree(hd_devp[i]);
        }
        class_destroy(huadeng_class);
#ifdef CHRDRV_OLD_STYLE
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
#endif
    }

    printk(KERN_EMERG "Testing message with KERN_EMERG severity level\n");
    printk(KERN_ALERT "Testing message with KERN_ALERT severity level\n");
    printk(KERN_CRIT "Testing message with KERN_CRIT severity level\n");
    printk(KERN_ERR "Testing message with KERN_ERR severity level\n");
    printk(KERN_WARNING "Testing message with KERN_WARNING severity level\n");
    printk(KERN_NOTICE "Testing message with KERN_NOTICE severity level\n");
    printk(KERN_INFO "Testing message with KERN_INFO severity level\n");
    printk(KERN_DEBUG "Testing message with KERN_DEBUG severity level\n");
}

module_init(llaolao_init);
module_exit(llaolao_exit);
module_param(hostip, charp, S_IRUGO);
module_param(targetip, charp, S_IRUGO);
module_param(hostport, ushort, S_IRUGO);
module_param(targetport, ushort, S_IRUGO);

MODULE_AUTHOR("GEDU lab");
MODULE_DESCRIPTION("LKM example - llaolao");
MODULE_LICENSE("GPL");
