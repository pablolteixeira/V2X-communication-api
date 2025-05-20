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
#include "sched_utils.h"

class PeriodicThread {
private:
    pthread_t thread;
    std::atomic<bool> running;
    std::atomic<__u64> period_ns;  // Period in nanoseconds
    std::atomic<__u64> runtime_ns; // Runtime in nanoseconds

    std::function<void ()> task_func;

    static void* threadFunction(void* arg) {
        PeriodicThread* self = static_cast<PeriodicThread*>(arg);
        self->executeThread();
        return NULL;
    }
    
    void executeThread() {
        struct pt_sched_attr attr;
        int ret;
        unsigned int flags = 0;
        
        //printf("Periodic thread started [%ld]\n", gettid());
        
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
            task_func();
            sched_yield();
            
            // Check if period has been updated
            __u64 current_period = period_ns.load();
            if (current_period != attr.sched_period) {
                attr.sched_period = attr.sched_deadline = current_period;
                ret = sched_setattr(0, &attr, flags);
                if (ret < 0) {
                    perror("sched_setattr(update)");
                }
            }
        }
        
        //printf("Periodic thread dies [%ld]\n", gettid());
    }
    
public:
    // Constructor
    PeriodicThread(std::function<void ()> function, __u64 period_microseconds, __u64 runtime_microseconds) 
        : running(false), task_func(function) {
            if(period_microseconds < 1000) {
                period_microseconds = 1000;
            }
            if (runtime_microseconds > period_microseconds) {
                runtime_microseconds = (unsigned long long)(period_microseconds * 0.5);
            }

            runtime_ns.store(runtime_microseconds * 1000);
            period_ns.store(period_microseconds * 1000);
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
        runtime_ns.store(new_period_microseconds * 1000 * 0.5);
    }
};

#endif // PERIODIC_THREAD_H