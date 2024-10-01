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
