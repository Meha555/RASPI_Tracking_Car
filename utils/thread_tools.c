#include "thread_tools.h"

int get_thread_policy(pthread_attr_t* attr) {
    int policy;
    int rs = pthread_attr_getschedpolicy(&attr, &policy);
    assert(rs == 0);
    switch (policy) {
        case SCHED_FIFO:
            printf("policy = SCHED_FIFO\n");
            break;
        case SCHED_RR:
            printf("policy = SCHED_RR\n");
            break;
        case SCHED_OTHER:
            printf("policy = SCHED_OTHER\n");
            break;
        default:
            printf("policy = UNKNOWN\n");
            break;
    }
    return policy;
}

int get_thread_priority(pthread_attr_t* attr) {
    struct sched_param param;

    int rs = pthread_attr_getschedparam(&attr, &param);
    assert(rs == 0);
    printf("priority = %d\n", param.__sched_priority);

    return param.__sched_priority;
}

void set_thread_policy(pthread_attr_t* attr, int policy) {
    int rs = pthread_attr_setschedpolicy(&attr, policy);
    assert(rs == 0);
    get_thread_policy(attr);
}