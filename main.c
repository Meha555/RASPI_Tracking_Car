#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "actuator/actuator.h"
#include "bcd/bcd.h"
#include "key/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "sonar/hc_sr04.h"
#include "utils/thread_tools.h".

#define THREAD_NUM 4
pthread_t tid[THREAD_NUM];
// pthread_attr_t tattr[THREAD_NUM];
// struct sched_param schedparam[THREAD_NUM];

// 全局变量的定义
sem_t sem_keyboard;           // 键盘资源信号量
sem_t sem_sonar;              // 声呐资源信号量
pthread_mutex_t mutex_param;  // 电机参数锁

void set_thread_attr(pthread_attr_t* tattr, int priority) {
    printf("Setting thread attribute.\n");
    pthread_attr_init(&tattr);
    if (get_thread_policy(&tattr) != SCHED_RR)
        set_thread_policy(&tattr, SCHED_RR);
    struct sched_param schedparam;
    schedparam.sched_priority = priority;
    pthread_attr_setschedparam(&tattr, &schedparam);
    get_thread_priority(&tattr);

    // pthread_attr_destroy(&tattr);
}

void* sonar_thread(void* args) {
    printf("In sonar_thread.\n");
    // pthread_attr_t tattr;
    // set_thread_attr(&tattr, 2);
    // pthread_attr_init(&tattr);
    // // if (get_thread_policy(&tattr) != SCHED_RR)
    // set_thread_policy(&tattr, SCHED_RR);
    // struct sched_param schedparam;
    // schedparam.sched_priority = 2;
    // pthread_attr_setschedparam(&tattr, &schedparam);
    // get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    tm1637_init();
    inital_hr024();
    int flag = 1;
    while (1) {
        int dist = get_distance();
        if (dist >= 400)
            printf("Lose precision!\n");
        else
            printf("Dectecting dist: %d CM\n", dist);
        flag ^= 1;
        bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, flag);
        pthread_mutex_lock(&mutex_param);
        param.dist = dist;  // 写入距离到参数地址
        pthread_mutex_unlock(&mutex_param);
        delay(500);
    }
    // pthread_attr_destroy(&tattr);
}

void* keyboard_action_thread(void* args) {
    printf("In keyboard_action_thread.\n");
    // pthread_attr_t tattr;
    // set_thread_attr(&tattr, 1);
    // pthread_attr_init(&tattr);
    // // if (get_thread_policy(&tattr) != SCHED_RR)
    // set_thread_policy(&tattr, SCHED_RR);
    // struct sched_param schedparam;
    // schedparam.sched_priority = 1;
    // pthread_attr_setschedparam(&tattr, &schedparam);
    // get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    unsigned char ch = 'E';
    while (1) {
        if (param.dist <= 5) {
            pthread_mutex_lock(&mutex_param);
            param.key_pressed = 'E';
            pthread_mutex_unlock(&mutex_param);
        } else {
            printf("--Before key_press--\n");
            ch = getchar();  // getchar()写锁外面，避免锁内阻塞
            // printf("--Get KEY: %c--\n", ch);
        }
        pthread_mutex_lock(&mutex_param);
        param.key_pressed = ch;
        printf("Submit key_pressed %c.\n", param.key_pressed);
        pthread_mutex_unlock(&mutex_param);
        sem_post(&sem_keyboard);
    }
    // pthread_attr_destroy(&tattr);
}

void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    // pthread_attr_t tattr;
    // set_thread_attr(&tattr, 0);
    // pthread_attr_init(&tattr);
    // // if (get_thread_policy(&tattr) != SCHED_RR)
    // set_thread_policy(&tattr, SCHED_RR);
    // struct sched_param schedparam;
    // schedparam.sched_priority = 0;
    // pthread_attr_setschedparam(&tattr, &schedparam);
    // get_thread_priority(&tattr);
    struct MotorParam param = *(struct MotorParam*)args;
    drive(&param);
    // pthread_attr_destroy(&tattr);
}

void* actuator_thread(void* args) {
    printf("In actuator_thread.\n");
    struct MotorParam param = *(struct MotorParam*)args;
    // pthread_mutex_lock(&mutex_param);
    // param.orient =
    // printf("Submit key_pressed %c.\n", param.key_pressed);
    // pthread_mutex_unlock(&mutex_param);
    initial_actuator();
    control_spin();
}

int main(int argc, char argv[]) {
    // 初始化互斥同步
    sem_init(&sem_sonar, 0, 1);
    sem_init(&sem_keyboard, 0, 1);
    pthread_mutex_init(&mutex_param, NULL);

    // 初始化参数
    struct MotorParam* motor_param = (struct MotorParam*)malloc(sizeof(struct MotorParam));
    motor_param->key_pressed = 'E';
    motor_param->dist = 400;
    motor_param->orient = AHEAD;

    // // 设置线程属性，将优先级设置为sonar_thread最高，keyboard_action_thread次之，motor_thread最低
    // for (int i = 0; i < THREAD_NUM; i++) {
    //     get_thread_policy(&tattr[i]);
    //     get_thread_priority(&tattr[i]);
    //     // pthread_attr_setschedpolicy(&tattr[i], SCHED_RR);
    //     // set_thread_attr(&tattr[i], THREAD_NUM - i);
    //     // get_thread_policy(&tattr[i]);
    //     // get_thread_priority(&tattr[i]);
    // }
    pthread_create(&tid[0], NULL, sonar_thread, motor_param);
    pthread_create(&tid[1], NULL, actuator_thread, motor_param);
    pthread_create(&tid[2], NULL, keyboard_action_thread, motor_param);
    pthread_create(&tid[3], NULL, motor_thread, motor_param);

    printf("==============\n");
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("Exiting main process...\n");

    // 销毁互斥同步
    sem_destroy(&sem_sonar);
    sem_destroy(&sem_keyboard);
    pthread_mutex_destroy(&mutex_param);

    // TODO - 添加清理引脚的代码

    return 0;
}