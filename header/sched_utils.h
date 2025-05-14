#ifndef SCHED_UTILS_H
#define SCHED_UTILS_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>

#define gettid() syscall(__NR_gettid)
#define SCHED_DEADLINE 6

// Define syscall numbers for different architectures
#ifdef __x86_64__
#define __NR_sched_setattr 314
#define __NR_sched_getattr 315
#endif

#ifdef __i386__
#define __NR_sched_setattr 351
#define __NR_sched_getattr 352
#endif

#ifdef __arm__
#define __NR_sched_setattr 380
#define __NR_sched_getattr 381
#endif

struct sched_attr {
    __u32 size;
    __u32 sched_policy;
    __u64 sched_flags;
    __s32 sched_nice;
    __u32 sched_priority;
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags);

#endif // SCHED_UTILS_H
