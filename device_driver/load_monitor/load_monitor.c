/*
 *  device char module
 *  
 *  (C) 2021.08.04 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/kallsyms.h>
#include <linux/tracepoint.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/stacktrace.h>

struct hrtimer timer;

static unsigned long *ptr_avenrun;

#define FSHIFT          11
#define FIXED_1         (1<<FSHIFT)
#define LOAD_INIT(x)    ((x) >> FSHIFT)
#define LOAD_FRAC(x)    LOAD_INIT((x) & (FIXED_1-1) * 100)

#define BACKTRACE_DEPTH 10

extern struct task_struct init_task;

#define next_task(p) \
        list_entry_rcu((p)->task.next, struct task_struct, tasks)

#define next_thread(p) \
        list_entry_rcu(p->thread_group.next, struct task_struct, thread_group)

#define do_each_thread(g, t) \
    for(g = t = &init_task; (g = t = next_task(g)) != &init_task;) do

#define while_each_thread(g, t) \
    while((t = next_thread(t)) != g)

static void  print_all_task_stack(void)
{
    struct task_struct *g, *p;
    unsigned long backtrace[BACKTRACE_DEPTH];
    struct stack_trace trace;

    memset(&trace, 0, sizeof(trace));
    memset(backtrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
    trace.max_entries = BACKTRACE_DEPTH;
    trace.entries = backtrace;

    printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk("\tLoad: %lu.%02lu, %lu.%02lu, %lu.%02lu\n",
           LOAD_INIT(ptr_avenrun[0]), LOAD_FRAC(ptr_avenrun[0]),
           LOAD_INIT(ptr_avenrun[1]), LOAD_FRAC(ptr_avenrun[1]),
           LOAD_INIT(ptr_avenrun[2]), LOAD_FRAC(ptr_avenrun[2])
           );

    rcu_read_lock()；

    printk("dump running task.\n");
    do_each_thread(g, p)
    {
        if(p->state == TASK_RUNNING)
        {
            printk("running task, comm: %s, pid %d\n", p->comm, p->pid);
            memset(&trace, 0, sizeof(trace));
            memset(backrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
            trace.max_entries = BACKTRACE_DEPTH;
            trace.entries = backtrace;
            save_stack_trace_tsk(p, &trace);
            print_stack_trace(&trace, 0);
        }
    } while_each_thread(g, p);

    printk("dump uninterrupted task.\n");
    do_each_thread(g, p)
    {
        if(p->state & TASK_UNINTERRUPTIBLE)
        {
            printk("uninterruptible task, comm: %s, pid %d\n", p->comm, p->pid);
            memset(&trace, 0, sizeof(trace));
            memset(backrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
            trace.max_entries = BACKTRACE_DEPTH;
            trace.entries = backtrace;
            save_stack_trace_tsk(p, &trace);
            print_stack_trace(&trace, 0);
        }
    }
    rcu_read_unlock();
}

static void check_load(void)
{
    static ktime_t last;
    u64 ms;
    intt load = LOAD_INIT(ptr_avenrun[0]);      

    if(load < 3)
        return;

    ms = ktime_to_ms(ktime_sub(ktime_get(), last));
    if(ms > 10 * 1000)
        return;

    last = ktime_get();
    print_all_task_stack();
}

static enum hrtimer_restart monitor_handler(struct hrtimer *hrtimer)
{
    enum hrtimer_restart ret = HRTIMER_RESTART;

    check_load();

    hrtimer_forward_now(hrtimer, ms_to_ktime(10));

    return ret;
}

static void start_timer(void)
{
    hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_PINNED);
    timer.function = monitor_handler;
    hrtimer_start_range_ns(&timer, ms_to_ktime(10), 0, HRTIMER_MODE_REL_PINNED);
}

static __init int load_monitor_init(void)
{
    ptr_avenrun = (void *)kallsyms_lookup_name("avenrun");
    if(!ptr_avenrun)
        return -EINVAL;

    start_timer();

    printk("load-monitor loaded.\n");

    return 0;
}

static __exit void load_monitor_exit(void)
{
    hrtimer_cancel(&timer);

    printk("laod-monitor unloaded.\n");
}


module_init(load_monitor_init);
module_exit(load_monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yinwg <hkdywg@163.com>");
MODULE_DESCRIPTION("cpu load monitor");

