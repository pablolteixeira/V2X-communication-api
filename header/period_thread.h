#ifndef PERIODIC_THREAD_H
#define PERIODIC_THREAD_H

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <atomic>

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

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags) {
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

class PeriodicThread {
private:
    pthread_t thread;
    std::atomic<bool> running;
    std::atomic<__u64> period_ns;  // Period in nanoseconds
    std::atomic<__u64> runtime_ns; // Runtime in nanoseconds
    std::function<void()> task;
    
    static void* threadFunction(void* arg) {
        PeriodicThread* self = static_cast<PeriodicThread*>(arg);
        self->executeThread();
        return NULL;
    }
    
    void executeThread() {
        struct sched_attr attr;
        int ret;
        unsigned int flags = 0;
        
        printf("Periodic thread started [%ld]\n", gettid());
        
        // Set scheduler attributes
        attr.size = sizeof(attr);
        attr.sched_flags = 0;
        attr.sched_nice = 0;
        attr.sched_priority = 0;
        attr.sched_policy = SCHED_DEADLINE;
        attr.sched_runtime = runtime_ns.load();
        attr.sched_period = attr.sched_deadline = period_ns.load();
        
        ret = sched_setattr(0, &attr, flags);
        if (ret < 0) {
            perror("sched_setattr");
            return;
        }
        
        // Execute the task periodically
        while (running.load()) {
            // Execute the provided function
            task();
            
            // Check if period has been updated
            __u64 current_period = period_ns.load();
            if (current_period != attr.sched_period) {
                attr.sched_period = attr.sched_deadline = current_period;
                ret = sched_setattr(0, &attr, flags);
                if (ret < 0) {
                    perror("sched_setattr (update)");
                }
            }
        }
        
        printf("Periodic thread dies [%ld]\n", gettid());
    }
    
public:
    // Constructor
    PeriodicThread(std::function<void()> function, __u64 period_microseconds, __u64 runtime_microseconds = 70 * 1000) 
        : running(false), runtime_ns(runtime_microseconds), task(function) {
            if(period_microseconds < 100) {
                period_microseconds = 100;
            }
            period_ns = period_microseconds * 1000;
    }
    
    // Destructor
    ~PeriodicThread() {
        stop();
    }
    
    // Start the thread
    bool start() {
        if (running.load()) {
            return false; // Already running
        }
        
        running.store(true);
        if (pthread_create(&thread, NULL, threadFunction, this) != 0) {
            running.store(false);
            perror("pthread_create");
            return false;
        }
        
        return true;
    }
    
    // Stop the thread
    void stop() {
        if (running.load()) {
            running.store(false);
            pthread_join(thread, NULL);
        }
    }
    
    // Update the period
    void update(const __u64 new_period_microseconds) {
        period_ns.store(new_period_microseconds * 1000);
    }
    
    // Update the runtime
    void updateRuntime(const __u64 new_period_microseconds) {
        runtime_ns.store(new_period_microseconds * 1000);
    }
};

#endif // PERIODIC_THREAD_H