#include <assert.h>       // assert()
#include <errno.h>        // EBUSY
#include <pthread.h>      // pthread相关函数
#include <stdio.h>        // printf()
#include <sys/syscall.h>  // syscall()
#include <unistd.h>       // R_OK

struct sub_param {
    pid_t* pid;
    int* loop;
};
typedef struct sub_param sub_param_t;

/**
 * @brief 子线程主方法
 */
void* sub_thd_func(void* ptr);

/**
 * @brief 检查线程是否存在的不同方法
 * @return 0表示线程已退出，1表示线程存活
 */
int check_if_thd_exist_1(pid_t pid, pid_t tid);
int check_if_thd_exist_2(pthread_t tid);
int check_if_thd_exist_3(pthread_t tid);

/**
 * @brief 创建一个非法的pthread_t，pid_t
 */
void gen_invalid_pthread_t(pthread_t* tid1, pid_t* tid2);

int main() {
    pid_t main_pid = getpid();
    printf("Main thread[%ld]\n", main_pid);

    // 创建子线程
    int sub_thd_loop = 1;
    pthread_t sub_tid;
    pid_t sub_tid2;
    sub_param_t param;
    param.loop = &sub_thd_loop;
    param.pid = &sub_tid2;

    assert(pthread_create(&sub_tid, NULL, sub_thd_func, &param) == 0);
    printf("Sub thread[%lu]\n", sub_tid);

    // 创建一个非法的pthread_t用于测试
    // 要先创建<invalid_tid>，再创建<sub_tid>，防止相同值被分配给<sub_tid>
    pthread_t invalid_tid1;
    pid_t invalid_tid2;
    gen_invalid_pthread_t(&invalid_tid1, &invalid_tid2);
    printf("Invalid thread[%lu] [%ld]\n", invalid_tid1, invalid_tid2);

    // 主线程接收console命令
    char q;
    do {
        q = getchar();
        if (q == 'q') {
            break;
        }

        int exist = 0;
        switch (q) {
            case '0':
                // 使用三种方法测试一个非法tid
                if (check_if_thd_exist_1(main_pid, invalid_tid2)) {
                    printf("check_if_thd_exist_1 -> Thread[%lu] is exist.\n",
                           invalid_tid2);
                }
                if (check_if_thd_exist_2(invalid_tid1)) {
                    printf("check_if_thd_exist_2 -> Thread[%lu] is exist.\n",
                           invalid_tid1);
                }
                if (check_if_thd_exist_3(invalid_tid1)) {
                    printf("check_if_thd_exist_3 -> Thread[%lu] is exist.\n",
                           invalid_tid1);
                }
                break;
            case '1':
                // 使用第一种方法测试
                exist = check_if_thd_exist_1(main_pid, sub_tid2);
                break;
            case '2':
                // 使用第二种方法测试
                exist = check_if_thd_exist_2(sub_tid);
                break;
            case '3':
                // 使用第三种方法测试
                exist = check_if_thd_exist_3(sub_tid);
                break;
        }

        if (exist == 1) {
            printf("Thread[%lu] is exist.\n", q == '1' ? sub_tid2 : sub_tid);
        }

        printf("\n");
    } while (1);

    sub_thd_loop = 0;
    pthread_join(sub_tid, NULL);

    return 0;
}

void* sub_thd_func(void* ptr) {
    sub_param_t* param = (sub_param_t*)ptr;
    *(param->pid) = syscall(SYS_gettid);

    int* loop = param->loop;
    const int duration_us = 1000 * 5;  // 5ms
    while (*loop) {
        usleep(duration_us);
    }
}

int check_if_thd_exist_1(pid_t pid, pid_t tid) {
    // 该方法要求使用pid_t，而不是pthread_t
    char fn[100];
    snprintf(fn, sizeof(fn), "/proc/%ld/task/%ld/stat", pid, tid);
    printf("[%s] Check if <%s> exist.\n", __FUNCTION__, fn);
    return access(fn, R_OK) == 0;
}

int check_if_thd_exist_2(pthread_t tid) {
    // 1. 0是pthread_kill第二个参数的保留值，用于查询线程是否还存活
    // 2. 某些平台上，pthread_kill传入0直接crash
    return pthread_kill(tid, 0) == 0;
}

int check_if_thd_exist_3(pthread_t tid) {
    // 在arm板上没有找到该函数，笔者ndk版本有16和23
    return pthread_tryjoin_np(tid, NULL) == EBUSY;
}

static pid_t g_tid;
void* func(void* ptr) {
    g_tid = syscall(SYS_gettid);
}

void gen_invalid_pthread_t(pthread_t* tid1, pid_t* tid2) {
    pthread_create(tid1, NULL, func, NULL);
    pthread_join(*tid1, NULL);
    *tid2 = g_tid;
}
