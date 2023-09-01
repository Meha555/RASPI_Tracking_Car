#include <pthread.h>
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <unistd.h>
#include "actuator/actuator.h"
#include "bcd/bcd.h"
#include "key/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "sonar/hc_sr04.h"
#include "utils/thread_tools.h"

#define THREAD_NUM 4
pthread_t tid[THREAD_NUM];

// 全局变量的定义
sem_t sem_keyboard;           // 键盘资源信号量
sem_t sem_sonar;              // 声呐资源信号量
pthread_mutex_t mutex_param;  // 电机参数锁

// 捕获CTRL+C
static void catch_sigint(int sig) {
    // 结束各个线程
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_cancel(tid[i]);
    }
    command_stop(0);   // motor
    command_reset(0);  // actuator
    // command_shut(0);   // buzzer
    signal(SIGINT, SIG_DFL);
    exit(0);  // 结束进程
}

// 超声波+舵机+数码管
void* sonar_thread(void* args) {
    printf("In sonar_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    tm1637_init();
    inital_hr024();
    initial_actuator();
    int flag = 1;
    while (1) {
        // 左转
        for (int i = -90; i < 90; i += 45) {
            // printf("Actuator spin left %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            delay(500);
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            flag ^= 1;
            bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, flag);
            pthread_mutex_lock(&mutex_param);
            param->dist = dist;  // 写入距离到参数地址
            pthread_mutex_unlock(&mutex_param);
        }
        // 右转
        for (int i = 90; i > -90; i -= 45) {
            // printf("Actuator spin right %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            delay(500);
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            flag ^= 1;
            bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, flag);
            pthread_mutex_lock(&mutex_param);
            param->dist = dist;  // 写入距离到参数地址
            pthread_mutex_unlock(&mutex_param);
        }
        // int dist = get_distance();
        // if (dist >= 400)
        //     printf("Lose precision!\n");
        // else
        //     printf("Dectecting dist: %d CM\n", dist);
        // flag ^= 1;
        // bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, flag);
        // pthread_mutex_lock(&mutex_param);
        // param.dist = dist;  // 写入距离到参数地址
        // pthread_mutex_unlock(&mutex_param);
        // delay(500);
    }
}

// 键盘动作
void* keyboard_action_thread(void* args) {
    printf("In keyboard_action_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    printf("Ready, waitting for command...\n");
    struct termios tms_old, tms_new;  // 声明终端属性结构体
    tcgetattr(0, &tms_old);           // 获取旧的终端属性
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    unsigned char ch = 'E';
    while (1) {
        if (param->dist <= 5) {
            pthread_mutex_lock(&mutex_param);
            printf("Emergency Break!\n");
            param->key_pressed = 'E';
            pthread_mutex_unlock(&mutex_param);
        } else {
            ch = getchar();  // getchar()写锁外面，避免锁内阻塞
            pthread_mutex_lock(&mutex_param);
            param->key_pressed = ch;
            printf("--Submit KEY: %c\n", param->key_pressed);
            pthread_mutex_unlock(&mutex_param);
        }
        sem_post(&sem_keyboard);
    }
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
}

// 电机
void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    drive(param);
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
    printf("Initializing...\n");
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    signal(SIGINT, catch_sigint);

    // 初始化互斥同步
    sem_init(&sem_sonar, 0, 1);
    sem_init(&sem_keyboard, 0, 0);
    pthread_mutex_init(&mutex_param, NULL);

    // 初始化参数
    struct MotorParam* motor_param = (struct MotorParam*)malloc(sizeof(struct MotorParam));
    motor_param->key_pressed = 'E';
    motor_param->dist = 400;
    motor_param->orient = AHEAD;

    pthread_create(&tid[0], NULL, sonar_thread, motor_param);
    // pthread_create(&tid[1], NULL, actuator_thread, motor_param);
    pthread_create(&tid[2], NULL, keyboard_action_thread, motor_param);
    pthread_create(&tid[3], NULL, motor_thread, motor_param);

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