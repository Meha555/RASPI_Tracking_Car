#include <pthread.h>
#include <stdio.h>
#include "bcd/bcd.h"
#include "key/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "sonar/hc_sr04.h"
#include "utils/thread_tools.h"

#define THREAD_NUM 3
pthread_t tid[THREAD_NUM];
// pthread_attr_t tattr[THREAD_NUM];
// struct sched_param schedparam[THREAD_NUM];

// void* bcd_thread(void* args) {
//     // printf("In bcd_thread.\n");
//     inital_hr024();
//     while (1) {
//         int dist = get_distance();
//         bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
//         delay(500);
//     }
// }

// 全局变量的定义
sem_t sem_keyboard;           // 键盘资源信号量
sem_t sem_sonar;              // 声呐资源信号量
pthread_mutex_t mutex_param;  // 电机参数锁

void* keyboard_action_thread(void* args) {
    printf("In keyboard_action_thread.\n");
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    // if (get_thread_policy(&tattr) != SCHED_RR)
    set_thread_policy(&tattr, SCHED_RR);
    struct sched_param schedparam;
    schedparam.sched_priority = 1;
    pthread_attr_setschedparam(&tattr, &schedparam);
    get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    while (1) {
        if (param.dist <= 5) {
            param.key_pressed = 'E';
        } else {
            pthread_mutex_lock(&mutex_param);
            param.key_pressed = getchar();
            pthread_mutex_unlock(&mutex_param);
        }
        printf("Submit key_pressed.\n");
        sem_post(&sem_keyboard);
    }
    pthread_attr_destroy(&tattr);
}

void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    // if (get_thread_policy(&tattr) != SCHED_RR)
    set_thread_policy(&tattr, SCHED_RR);
    struct sched_param schedparam;
    schedparam.sched_priority = 0;
    pthread_attr_setschedparam(&tattr, &schedparam);
    get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    drive(&param);
    pthread_attr_destroy(&tattr);
}

void* sonar_thread(void* args) {
    printf("In sonar_thread.\n");
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    // if (get_thread_policy(&tattr) != SCHED_RR)
    set_thread_policy(&tattr, SCHED_RR);
    struct sched_param schedparam;
    schedparam.sched_priority = 2;
    pthread_attr_setschedparam(&tattr, &schedparam);
    get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    inital_hr024();
    while (1) {
        int dist = get_distance();
        if (dist >= 400)
            printf("Lose precision!\n");
        else
            printf("Dectecting dist: %d CM\n", dist);
        bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
        pthread_mutex_lock(&mutex_param);
        param.dist = dist;  // 写入距离到参数地址
        pthread_mutex_unlock(&mutex_param);
        delay(500);
    }
    pthread_attr_destroy(&tattr);
}

int main(int argc, char argv[]) {
    // 初始化互斥同步
    sem_init(&sem_sonar, 0, 1);
    sem_init(&sem_keyboard, 0, 1);
    pthread_mutex_init(&mutex_param, NULL);

    // 初始化参数
    struct MotorParam motor_param = {
        .key_pressed = 'E',
        .dist = 400,
        .orient = AHEAD,
    };

    pthread_create(&tid[0], NULL, sonar_thread, &motor_param);
    pthread_create(&tid[1], NULL, keyboard_action_thread, &motor_param);
    pthread_create(&tid[2], NULL, motor_thread, &motor_param);

    printf("==============\n");
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("Exiting main process...\n");

    // 销毁互斥同步
    sem_destroy(&sem_sonar);
    sem_destroy(&sem_keyboard);
    pthread_mutex_destroy(&mutex_param);

    return 0;
}