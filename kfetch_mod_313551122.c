#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/utsname.h>
#include <linux/sched/signal.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/version.h>

#define KFETCH_DEV_NAME "kfetch"
#define KFETCH_BUF_SIZE 1024
#define LINUX_LOGO_LINES 7

#define KFETCH_RELEASE   (1 << 0)
#define KFETCH_NUM_CPUS  (1 << 1)
#define KFETCH_CPU_MODEL (1 << 2)
#define KFETCH_MEM       (1 << 3)
#define KFETCH_UPTIME    (1 << 4)
#define KFETCH_NUM_PROCS (1 << 5)

#define KFETCH_FULL_INFO ((1 << 6) - 1)

static char *kfetch_buf;
static int info_mask = KFETCH_FULL_INFO;
static int major_number;
static struct class *cls;
static DEFINE_MUTEX(kfetch_mutex);

static ssize_t kfetch_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset){
    /* fetching the information */
    int buf_len = 0;
    struct sysinfo si;
    struct task_struct *task;
    int proc_count = 0;
    int line = 0;
    char linux_logo[7][50] = {
        "         .-.        ",
        "        (.. |       ",
        "        \033[33m<>\033[0m  |       ",
        "       / --- \\      ",
        "      ( |   | )     ",
        "    \033[33m|\\\033[0m\\_)__(_/\033[33m/|\033[0m    ",
        "   \033[33m<__)\033[0m------\033[33m(__>\033[0m   "
    };

    if (*offset != 0)
        return 0;

    // mutex_lock(&kfetch_mutex);

    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"                    ");
    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"%s\n", utsname()->nodename);

    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"----------------------------------------\n");

    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    if (info_mask & KFETCH_RELEASE) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"Kernel: %s\n", utsname()->release);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    if (info_mask & KFETCH_CPU_MODEL) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"CPU: %s\n",boot_cpu_data.x86_model_id);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    if (info_mask & KFETCH_NUM_CPUS) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"CPUs: %d / %d\n", num_online_cpus(), num_possible_cpus());
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    if (info_mask & KFETCH_MEM) {
        si_meminfo(&si);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"Mem: %lu MB / %lu MB\n",si.freeram >> 10, si.totalram >> 10);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    if (info_mask & KFETCH_NUM_PROCS) {
        for_each_process(task)
            proc_count++;
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"Procs: %d\n", proc_count);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    if (info_mask & KFETCH_UPTIME) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"Uptime: %lu mins\n", (unsigned long) jiffies_to_msecs(get_jiffies_64()) / 60000);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    while(line <= LINUX_LOGO_LINES){
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,"\n");
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,linux_logo[line++]);
    }

    // mutex_unlock(&kfetch_mutex);

    if (copy_to_user(buffer, kfetch_buf, buf_len)){
        pr_alert("Failed to copy data to user");
        return 0;
    }

    /* cleaning up */
    *offset = buf_len;
    return buf_len;
}


static ssize_t kfetch_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset){
    int mask_info;

    if (copy_from_user(&mask_info, buffer, length)) {
        pr_alert("Failed to copy data from user");
        return 0;
    }

    /* setting the information mask */
    // mutex_lock(&kfetch_mutex);
    info_mask = mask_info;
    // mutex_unlock(&kfetch_mutex);
    return 0;
}


static int kfetch_open(struct inode *inode, struct file *file) {
    if (!mutex_trylock(&kfetch_mutex)) {
        pr_alert("Device is currently in use by another process.\n");
        return -EBUSY;
    }
    try_module_get(THIS_MODULE);
    pr_info("Device opened successfully.\n");
    return 0;
}

static int kfetch_release(struct inode *inode, struct file *file) {
    mutex_unlock(&kfetch_mutex);
    module_put(THIS_MODULE);
    pr_info("Device released successfully.\n");
    return 0;
}

static struct file_operations kfetch_ops = {
    .owner = THIS_MODULE,
    .read = kfetch_read,
    .write = kfetch_write,
    .open = kfetch_open,
    .release = kfetch_release,
};

static int __init kfetch_init(void) {
    major_number = register_chrdev(0, KFETCH_DEV_NAME, &kfetch_ops);
    if (major_number < 0) {
        pr_alert("Failed to register device: %d\n", major_number);
        return major_number;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    cls = class_create(KFETCH_DEV_NAME);
#else
    cls = class_create(THIS_MODULE, KFETCH_DEV_NAME);
#endif
    if (IS_ERR(cls)) {
        unregister_chrdev(major_number, KFETCH_DEV_NAME);
        return PTR_ERR(cls);
    }

    if (device_create(cls, NULL, MKDEV(major_number, 0), NULL, KFETCH_DEV_NAME) == NULL) {
        pr_alert("Failed to create device\n");
        class_destroy(cls);
        unregister_chrdev(major_number, KFETCH_DEV_NAME);
        return -1;
    }

    kfetch_buf = kmalloc(KFETCH_BUF_SIZE, GFP_KERNEL);
    if (!kfetch_buf) {
        pr_alert("Failed to allocate memory\n");
        device_destroy(cls, MKDEV(major_number, 0));
        class_destroy(cls);
        unregister_chrdev(major_number, KFETCH_DEV_NAME);
        return -ENOMEM;
    }

    pr_info("kfetch module loaded successfully with major number %d\n", major_number);
    return 0;
}


static void __exit kfetch_exit(void) {
    kfree(kfetch_buf);
    device_destroy(cls, MKDEV(major_number, 0));
    class_destroy(cls);
    unregister_chrdev(major_number, KFETCH_DEV_NAME);
    pr_info("kfetch module unloaded\n");
}

module_init(kfetch_init);
module_exit(kfetch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("313551122");
MODULE_DESCRIPTION("system information fetching with a linux logo");
