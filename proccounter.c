#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include "proccounter.h"
#define MODULE_NAME "proccounter"

static unsigned long long do_execveat_count = 0;
static unsigned long long kthread_create_count = 0;
static unsigned long long copy_process_count = 0;

static struct kprobe copy_process_probe = { .symbol_name = "copy_process", };
static struct kprobe do_execveat_probe = { .symbol_name = "do_execveat_common", };
static struct kprobe do_kthread_create_probe = { .symbol_name = "__kthread_create_on_node", };
static struct proc_ops proc_entry = { .proc_read = read_proc_entry, };


static ssize_t read_proc_entry(struct file* filep, char* buffer, size_t len, loff_t* offset) {
    char* buf = kvzalloc(512, GFP_USER);
    ssize_t ret;

    if (!buf) {
        return -ENOMEM;
    }
    
    snprintf(buf, 500, "Total execve() Count: %llu\nTotal kthread_create() Count: %llu\nTotal fork() Count: %llu\n", do_execveat_count, kthread_create_count, copy_process_count);
    ret = simple_read_from_buffer(buffer, len, offset, buf, strlen(buf));
    kvfree(buf);
    return ret;
}

static ssize_t init_proc_entries(void) {
    if (proc_create("proccounter", 0444, NULL, &proc_entry) == NULL) {
        remove_proc_entries();
        return -ENOMEM;
    }

    return 0;
}

static void remove_proc_entries(void) {
    remove_proc_entry("proccounter", NULL);
}



static int do_execveat_pre(struct kprobe* kp, struct pt_regs* regs) {
    do_execveat_count++;
    return 0;
}

static int do_kthread_create_pre(struct kprobe* kp, struct pt_regs* regs) {
    kthread_create_count++;
    return 0;
}

static int copy_process_pre(struct kprobe* kp, struct pt_regs* regs) {
    copy_process_count++;
    return 0;
}

static void remove_kprobes(void) {
    unregister_kprobe(&copy_process_probe);
    unregister_kprobe(&do_execveat_probe);
    unregister_kprobe(&do_kthread_create_probe);
}

static int init_kprobes(void) {
    int ret = 0;

    do_execveat_probe.pre_handler = do_execveat_pre;
    do_execveat_probe.post_handler = NULL;
    do_execveat_probe.fault_handler = NULL;

    ret = register_kprobe(&do_execveat_probe);
    if (ret) {
        remove_kprobes();
        return ret;
    }

    do_kthread_create_probe.pre_handler = do_kthread_create_pre;
    do_kthread_create_probe.post_handler = NULL;
    do_kthread_create_probe.fault_handler = NULL;

    ret = register_kprobe(&do_kthread_create_probe);
    if (ret) {
        remove_kprobes();
        return ret;
    }

    copy_process_probe.pre_handler = copy_process_pre;
    copy_process_probe.post_handler = NULL;
    copy_process_probe.fault_handler = NULL;

    ret = register_kprobe(&copy_process_probe);
    if (ret) {
	remove_kprobes();
	return ret;
    }

    return ret;
}



static int __init insert_module(void) {
    int ret = init_kprobes();

    if (ret) {
        printk(KERN_ERR "Failed to insert probes with return message %d", ret);
        return ret;
    }

    ret = init_proc_entries();

    if (ret) {
        printk(KERN_ERR "Failed to register procfs entries.");
        return ret;
    }

    printk(KERN_INFO "Kprobe planted and ready.");
    return 0;

}

static void __exit remove_module(void) {
    remove_kprobes();
    remove_proc_entries();
    printk("Kprobe removed.");
    return;
}

module_init(insert_module);
module_exit(remove_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sudhip Nashi");
MODULE_DESCRIPTION("counter for processes and threads");

