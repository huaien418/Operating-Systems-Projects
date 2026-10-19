#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>   // copy_to_user, copy_from_user
#include <linux/utsname.h>   // Hostname, Kernel release
#include <linux/mm.h>        // Memory
#include <linux/sched/signal.h> // Process count
#include <linux/timekeeping.h>  // Uptime
#include <linux/cpumask.h>      // CPU count
#include <linux/mutex.h>        // Mutex
#include <linux/of.h>           // Device Tree (for CPU Model)

#define DEVICE_NAME "kfetch"
#define CLASS_NAME  "kfetch_class"
#define KFETCH_BUF_SIZE 1024

// Mask Definitions (from kfetch.h)
#define KFETCH_RELEASE   (1 << 0)
#define KFETCH_NUM_CPUS  (1 << 1)
#define KFETCH_CPU_MODEL (1 << 2)
#define KFETCH_MEM       (1 << 3)
#define KFETCH_UPTIME    (1 << 4)
#define KFETCH_NUM_PROCS (1 << 5)
#define KFETCH_FULL_INFO ((1 << 6) - 1)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huaien418");
MODULE_DESCRIPTION("Assignment 3 kfetch module");

// Global Variables
static int major;
static struct class *kfetch_class = NULL;
static struct device *kfetch_device = NULL;
static int kfetch_mask = KFETCH_FULL_INFO; // Default: Show all
static char kfetch_buf[KFETCH_BUF_SIZE];   // Buffer for output
static DEFINE_MUTEX(kfetch_mutex);         // Mutex for thread safety

// Logo ASCII Art
static const char *logo_lines[] = {
    "        .-.        ",
    "       (.. |       ",
    "       <>  |       ",
    "      / --- \\      ",
    "     ( |   | )      ",
    "   |\\\\_)__(_//|    ",
    "  <__)------(__>   "
};
#define LOGO_LINES_NUM 7

// File Operations
static int kfetch_open(struct inode *inode, struct file *file) {
    return 0;
}

static int kfetch_release(struct inode *inode, struct file *file) {
    return 0;
}

// WRITE: Update the mask
static ssize_t kfetch_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
    int mask_info;

    if (length != sizeof(int))
        return -EINVAL;

    if (copy_from_user(&mask_info, buffer, length))
        return -EFAULT;

    mutex_lock(&kfetch_mutex);
    kfetch_mask = mask_info;
    mutex_unlock(&kfetch_mutex);

    return length;
}

// READ: Generate and return the info string
static ssize_t kfetch_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    if (*offset > 0) return 0; // End of file

    mutex_lock(&kfetch_mutex);

    // 1. Fetch System Info
    struct new_utsname *uts = init_utsname(); // Hostname, Release
    struct sysinfo si;
    si_meminfo(&si); // Memory
    struct timespec64 uptime;
    ktime_get_boottime_ts64(&uptime); // Uptime
    
    // Count Processes
    struct task_struct *task;
    int procs = 0;
    for_each_process(task) { procs++; }

    // Get CPU Model (from Device Tree)
    const char *cpu_model = "RISC-V Processor";
    struct device_node *np = of_find_node_by_path("/cpus/cpu@0");
    if (np) {
        if (of_property_read_string(np, "model", &cpu_model) != 0) {
             of_property_read_string(np, "compatible", &cpu_model);
        }
        of_node_put(np);
    }

    // 2. Prepare Info Lines
    char info_lines[6][64];
    int info_count = 0;

    // Hostname and Separator (Always shown first)
    char hostname_line[64];
    snprintf(hostname_line, 64, "%s", uts->nodename);
    
    char separator_line[64];
    memset(separator_line, '-', strlen(uts->nodename));
    separator_line[strlen(uts->nodename)] = '\0';

    // Optional Info based on Mask
    if (kfetch_mask & KFETCH_RELEASE)
        snprintf(info_lines[info_count++], 64, "Kernel:   %s", uts->release);
    if (kfetch_mask & KFETCH_CPU_MODEL)
        snprintf(info_lines[info_count++], 64, "CPU:      %s", cpu_model);
    if (kfetch_mask & KFETCH_NUM_CPUS)
        snprintf(info_lines[info_count++], 64, "CPUs:     %d / %d", num_online_cpus(), num_possible_cpus());
    if (kfetch_mask & KFETCH_MEM)
        snprintf(info_lines[info_count++], 64, "Mem:      %lu MB / %lu MB", 
                 (si.freeram * si.mem_unit) / 1024 / 1024, 
                 (si.totalram * si.mem_unit) / 1024 / 1024);
    if (kfetch_mask & KFETCH_NUM_PROCS)
        snprintf(info_lines[info_count++], 64, "Procs:    %d", procs);
    if (kfetch_mask & KFETCH_UPTIME)
        snprintf(info_lines[info_count++], 64, "Uptime:   %llu mins", uptime.tv_sec / 60);

    // 3. Format Output (Side-by-side)
    char *buf_ptr = kfetch_buf;
    memset(kfetch_buf, 0, KFETCH_BUF_SIZE);

    int total_lines = (LOGO_LINES_NUM > (2 + info_count)) ? LOGO_LINES_NUM : (2 + info_count);

    for (int i = 0; i < total_lines; i++) {
        // Left side: Logo
        if (i < LOGO_LINES_NUM)
            buf_ptr += sprintf(buf_ptr, "%-20s", logo_lines[i]);
        else
            buf_ptr += sprintf(buf_ptr, "%-20s", ""); // Padding if logo ended

        // Right side: Info
        if (i == 0)
            buf_ptr += sprintf(buf_ptr, "%s\n", hostname_line);
        else if (i == 1)
            buf_ptr += sprintf(buf_ptr, "%s\n", separator_line);
        else if (i - 2 < info_count)
            buf_ptr += sprintf(buf_ptr, "%s\n", info_lines[i - 2]);
        else
            buf_ptr += sprintf(buf_ptr, "\n");
    }

    // 4. Copy to User
    int data_len = buf_ptr - kfetch_buf;
    int ret = 0;
    
    if (copy_to_user(buffer, kfetch_buf, data_len)) {
        ret = -EFAULT;
    } else {
        ret = data_len;
        *offset += data_len;
    }

    mutex_unlock(&kfetch_mutex);
    return ret;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = kfetch_open,
    .release = kfetch_release,
    .read = kfetch_read,
    .write = kfetch_write,
};

static int __init kfetch_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;

    kfetch_class = class_create(CLASS_NAME);
    if (IS_ERR(kfetch_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(kfetch_class);
    }

    kfetch_device = device_create(kfetch_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(kfetch_device)) {
        class_destroy(kfetch_class);
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(kfetch_device);
    }

    printk(KERN_INFO "kfetch_mod: loaded\n");
    return 0;
}

static void __exit kfetch_exit(void) {
    device_destroy(kfetch_class, MKDEV(major, 0));
    class_destroy(kfetch_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "kfetch_mod: unloaded\n");
}

module_init(kfetch_init);
module_exit(kfetch_exit);
