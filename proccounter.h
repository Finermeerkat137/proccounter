static ssize_t read_proc_entry(struct file*, char*, size_t, loff_t*);
static ssize_t init_proc_entries(void);
static void remove_proc_entries(void);
static int do_execveat_pre(struct kprobe*, struct pt_regs*);
static int do_kthread_create_pre(struct kprobe*, struct pt_regs*);
static void remove_kprobes(void);
static int init_kprobes(void);