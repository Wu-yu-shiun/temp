#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE2(sys_revstr, char *, str, size_t, len) {
    char kernel_buffer[256];
    
    // 複製字串從使用者空間到核心空間
    if (copy_from_user(kernel_buffer, str, len)) {
        return -EFAULT;
    }

    printk(KERN_INFO "The origin string: %s\n", kernel_buffer);

    // 反轉字串
    int i;
    for (i = 0; i < len / 2; i++) {
        char temp = kernel_buffer[i];
        kernel_buffer[i] = kernel_buffer[len - i - 1];
        kernel_buffer[len - i - 1] = temp;
    }

    printk(KERN_INFO "The reversed string: %s\n", kernel_buffer);

    // 將結果複製回使用者空間
    if (copy_to_user(str, kernel_buffer, len)) {
        return -EFAULT;
    }

    return 0;
}


//////
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/slab.h>

SYSCALL_DEFINE2(revstr, int, length, char __user *, str) {
    // 動態分配記憶體，用於儲存來自用戶空間的字串
    char *buffer = kmalloc(sizeof(char) * (length + 1), GFP_KERNEL);
    int i, j;
    unsigned long len = length;

    // 檢查記憶體是否分配成功
    if (!buffer)
        return -ENOMEM;

    // 從用戶空間複製字串到內核空間
    if (copy_from_user(buffer, str, len + 1)) {
        kfree(buffer);
        return -EFAULT;
    }

    // 打印原始字串到內核ring buffer，dmesg可查看
    printk(KERN_INFO "The origin string: %s\n", buffer);

    // 反轉字串
    for (i = 0, j = length - 1; i < j; i++, j--) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // 打印反轉後的字串到內核ring buffer
    printk(KERN_INFO "The reversed string: %s\n", buffer);

    // 釋放分配的記憶體
    kfree(buffer);

    // 返回 0 表示成功
    return 0;
}

//
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE2(sys_revstr, char __user *, str, size_t, len) {
    char *buffer;
    int i;
    buffer = kmalloc(sizeof(char) * (len + 1), GFP_KERNEL);
    if (!buffer) {
        return -ENOMEM;
    }
    if (copy_from_user(buffer, str, len)) {
        kfree(buffer);
        return -EFAULT;
    }
    buffer[len] = '\0';
    printk(KERN_INFO "The origin string: %s\n", buffer);

    // reverse string
    for (i = 0; i < len / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[len - i - 1];
        buffer[len - i - 1] = temp;
    }
    printk(KERN_INFO "The reversed string: %s\n", buffer);

    if (copy_to_user(str, buffer, len)) {
        kfree(buffer);
        return -EFAULT;
    }
    kfree(buffer);
    return 0;
}
