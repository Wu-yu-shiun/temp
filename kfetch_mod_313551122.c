#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>



#define KFETCH_NUM_INFO      6

#define KFETCH_RELEASE      (1 << 0)
#define KFETCH_NUM_CPUS     (1 << 1)
#define KFETCH_CPU_MODEL    (1 << 2)
#define KFETCH_MEM          (1 << 3)
#define KFETCH_UPTIME       (1 << 4)
#define KFETCH_NUM_PROCS    (1 << 5)

#define KFETCH_FULL_INFO    ((1 << KFETCH_NUM_INFO) - 1)
