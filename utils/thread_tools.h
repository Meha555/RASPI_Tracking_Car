#ifndef THREAD_TOOLS_H
#define THREAD_TOOLS_H

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

extern int get_thread_policy(pthread_attr_t* attr);
extern int get_thread_priority(pthread_attr_t* attr);
extern void set_thread_policy(pthread_attr_t* attr, int policy);
extern void set_thread_attr(pthread_attr_t* tattr, int priority);

#endif  // THREAD_TOOLS_H