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

#define KFETCH_RELEASE   (1 << 0)
#define KFETCH_NUM_CPUS  (1 << 1)
#define KFETCH_CPU_MODEL (1 << 2)
#define KFETCH_MEM       (1 << 3)
#define KFETCH_UPTIME    (1 << 4)
#define KFETCH_NUM_PROCS (1 << 5)

#define KFETCH_FULL_INFO ((1 << 6) - 1)

// 全域變數
static char *kfetch_buf;
static int info_mask = KFETCH_FULL_INFO;
static int major_number;
static struct class *cls;
static DEFINE_MUTEX(kfetch_mutex); // 使用 mutex 處理多線程安全

// 設備文件操作 - read
static ssize_t kfetch_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset) {
    int buf_len = 0;
    struct sysinfo si;
    struct task_struct *task;
    int proc_count = 0;

    if (*offset != 0) // 單次讀取
        return 0;

    mutex_lock(&kfetch_mutex);

    // 輸出 logo，\033[33m變黃，\033[0m變白
    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                         "      .-.     \n"
                         "     (.. |    \n"
                         "     \033[33m<>\033[0m  |    \n"
                         "    / --- \   \n"
                         "   ( |   | )  \n"
                         " \033[33m|\\033[0m\_)__(_/\033[33m/|\033[0m \n"
                         "\033[33m<__)\033[0m------\033[33m(__>\033[0m\n");

    // 加入 hostname 和分隔線
    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                         "%s\n", utsname()->nodename);
    buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                         "--------------------------\n");

    // 根據 info_mask 選擇性輸出資訊
    if (info_mask & KFETCH_RELEASE) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "Kernel: %s\n", utsname()->release);
    }

    if (info_mask & KFETCH_CPU_MODEL) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "CPU: QEMU Virtual CPU (example)\n");
    }

    if (info_mask & KFETCH_NUM_CPUS) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "CPUs: %d / %d\n", num_online_cpus(), num_possible_cpus());
    }

    if (info_mask & KFETCH_MEM) {
        si_meminfo(&si);
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "Mem: %lu MB / %lu MB\n",
                             si.freeram >> 10, si.totalram >> 10);
    }

    if (info_mask & KFETCH_NUM_PROCS) {
        for_each_process(task)
            proc_count++;
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "Procs: %d\n", proc_count);
    }

    if (info_mask & KFETCH_UPTIME) {
        buf_len += scnprintf(kfetch_buf + buf_len, KFETCH_BUF_SIZE - buf_len,
                             "Uptime: %lu mins\n", (unsigned long) jiffies_to_msecs(get_jiffies_64()) / 60000);
    }

    mutex_unlock(&kfetch_mutex);

    if (copy_to_user(user_buf, kfetch_buf, buf_len))
        return -EFAULT;

    *offset = buf_len; // 更新偏移量
    return buf_len;
}

// 設備文件操作 - write
static ssize_t kfetch_write(struct file *file, const char __user *user_buf, size_t len, loff_t *offset) {
    int mask_info;

    if (len != sizeof(int))
        return -EINVAL;

    if (copy_from_user(&mask_info, user_buf, len))
        return -EFAULT;

    mutex_lock(&kfetch_mutex);
    info_mask = mask_info; // 設定 info_mask
    mutex_unlock(&kfetch_mutex);

    return len;
}

// 設備文件操作 - open 和 release
static int kfetch_open(struct inode *inode, struct file *file) {
    return 0;
}

static int kfetch_release(struct inode *inode, struct file *file) {
    return 0;
}

// 定義 file_operations
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
    cls = class_create(KFETCH_DEV_NAME); // 較新版本只需提供名稱
#else
    cls = class_create(THIS_MODULE, KFETCH_DEV_NAME); // 舊版本需要指定模組
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
    kfree(kfetch_buf); // 釋放緩衝區
    device_destroy(cls, MKDEV(major_number, 0));
    class_destroy(cls);
    unregister_chrdev(major_number, KFETCH_DEV_NAME);
    pr_info("kfetch module unloaded\n");
}

module_init(kfetch_init);
module_exit(kfetch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("313551122");
MODULE_DESCRIPTION("system information fetching and a linux logo");



