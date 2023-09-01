#include "motor.h"
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#define PWM_MAX 30    // PWM占空比
#define PWM_DELTA 10  // PWM每次增量

int curr_speed[2] = {0};

enum Direct {
    RIGHT,
    LEFT,
    FORWARD,
    BACKWARD,
    STOP
} g_gear = STOP;   // 当前档位
int CAR_HEAD = 1;  // 车头方向

struct Motor motor[2] = {
    {.m1 = 26, .m2 = 27},
    {.m1 = 29, .m2 = 28},
};

void command_stop(int sig) {
    for (int i = 0; i < 2; i++) {
        digitalWrite(motor[i].m1, LOW);
        digitalWrite(motor[i].m2, LOW);
        pinMode(motor[i].m1, INPUT);
        pinMode(motor[i].m2, INPUT);
    }
    // signal(SIGINT, SIG_DFL);  // 重新注册回默认信号处理程序
}
void inital_drive() {
    for (int i = 0; i < 2; i++) {
        pinMode(motor[i].m1, OUTPUT);
        pinMode(motor[i].m2, OUTPUT);
        softPwmCreate(motor[i].m1, 0, 100);  // 创建一个软件控制的 PWM 管脚【将m1设置为PWM控制】
        softPwmCreate(motor[i].m2, 0, 100);  // 创建一个软件控制的 PWM 管脚【将m2设置为PWM控制】
    }
    // signal(SIGINT, command_stop);  // 注册我们的CTRL+C停止点击的信号处理程序
}
static void cleanup_pin() {
    for (int i = 0; i < 2; i++) {
        pinMode(motor[i].m1, INPUT);
        pinMode(motor[i].m2, INPUT);
    }
}
void bothside_ahead_speedup() {
    for (; curr_speed[LEFT] < PWM_MAX && curr_speed[RIGHT] < PWM_MAX; curr_speed[LEFT] += PWM_DELTA, curr_speed[RIGHT] += PWM_DELTA) {
        softPwmWrite(motor[LEFT].m1, curr_speed[LEFT]);
        softPwmWrite(motor[RIGHT].m1, curr_speed[RIGHT]);
        delay(200);
    }
}
void bothside_ahead_slowdown() {
    for (; curr_speed[LEFT] > 0 && curr_speed[RIGHT] > 0; curr_speed[LEFT] -= PWM_DELTA, curr_speed[RIGHT] -= PWM_DELTA) {
        softPwmWrite(motor[LEFT].m1, curr_speed[LEFT]);
        softPwmWrite(motor[RIGHT].m1, curr_speed[RIGHT]);
        delay(200);
    }
    softPwmWrite(motor[LEFT].m1, 0);
    softPwmWrite(motor[RIGHT].m1, 0);
}
void bothside_goback_speedup() {
    for (; curr_speed[LEFT] < PWM_MAX && curr_speed[RIGHT] < PWM_MAX; curr_speed[LEFT] += PWM_DELTA, curr_speed[RIGHT] += PWM_DELTA) {
        softPwmWrite(motor[LEFT].m2, curr_speed[LEFT]);
        softPwmWrite(motor[RIGHT].m2, curr_speed[RIGHT]);
        delay(200);
    }
}
void bothside_goback_slowdown() {
    for (; curr_speed[LEFT] > 0 && curr_speed[RIGHT] > 0; curr_speed[LEFT] -= PWM_DELTA, curr_speed[RIGHT] -= PWM_DELTA) {
        softPwmWrite(motor[LEFT].m2, curr_speed[LEFT]);
        softPwmWrite(motor[RIGHT].m2, curr_speed[RIGHT]);
        delay(200);
    }
    softPwmWrite(motor[LEFT].m2, 0);
    softPwmWrite(motor[RIGHT].m2, 0);
}
void onside_ahead_speedup(enum Direct direct) {
    for (; curr_speed[direct] < PWM_MAX; curr_speed[direct] += PWM_DELTA) {
        softPwmWrite(motor[direct].m1, curr_speed[direct]);
        delay(200);
    }
}
void onside_goback_speedup(enum Direct direct) {
    for (; curr_speed[direct] < PWM_MAX; curr_speed[direct] += PWM_DELTA) {
        softPwmWrite(motor[direct].m2, curr_speed[direct]);
        delay(200);
    }
}
void onside_ahead_slowdown(enum Direct direct) {
    for (; curr_speed[direct] > 0; curr_speed[direct] -= PWM_DELTA) {
        softPwmWrite(motor[direct].m1, curr_speed[direct]);
        delay(200);
    }
    softPwmWrite(motor[direct].m1, 0);
}
void onside_goback_slowdown(enum Direct direct) {
    for (; curr_speed[direct] > 0; curr_speed[direct] -= PWM_DELTA) {
        softPwmWrite(motor[direct].m2, curr_speed[direct]);
        delay(200);
    }
    softPwmWrite(motor[direct].m2, 0);
}
void drive_break() {
    for (int i = 0; i < 2; i++) {
        softPwmWrite(motor[i].m1, 0);
        softPwmWrite(motor[i].m2, 0);
        curr_speed[i] = 0;
    }
}
void drive(struct MotorParam* param) {
    inital_drive();
    int dist = 0;
    unsigned char ch = 'E';
    while (1) {
        sem_wait(&sem_keyboard);  // 等待键盘动作
        pthread_mutex_lock(&mutex_param);
        ch = param->key_pressed;
        printf("--Get KEY: %c--\n", ch);
        printf("--Get DIST: %d--\n", param->dist);
        printf("--Get ORIENT: %d--\n", param->orient);
        pthread_mutex_unlock(&mutex_param);
        switch (ch) {
            case 'W': {  // 前进
                printf("Forward~\n");
                CAR_HEAD = 1;
                printf("Current head: %d\n", CAR_HEAD);
                printf("=>Before Gear: %d\n", g_gear);
                if (g_gear == BACKWARD)
                    bothside_goback_slowdown();
                else if (g_gear == LEFT)
                    onside_goback_slowdown(RIGHT);
                else if (g_gear == RIGHT)
                    onside_goback_slowdown(LEFT);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                g_gear = FORWARD;
                bothside_ahead_speedup();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'A': {  // 左转
                printf("Turn Left~\n");
                printf("Current head: %d\n", CAR_HEAD);
                printf("=>Before Gear: %d\n", g_gear);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                if (CAR_HEAD == 1) {
                    if (g_gear != LEFT) {
                        onside_ahead_slowdown(LEFT);
                    }
                    onside_ahead_speedup(RIGHT);
                } else {
                    if (g_gear != LEFT) {
                        onside_goback_slowdown(LEFT);
                    }
                    onside_goback_speedup(RIGHT);
                }
                g_gear = LEFT;
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'S': {  // 后退
                printf("Backward~\n");
                CAR_HEAD = 0;
                printf("Current head: %d\n", CAR_HEAD);
                printf("=>Before Gear: %d\n", g_gear);
                if (g_gear == FORWARD)
                    bothside_ahead_slowdown();
                else if (g_gear == LEFT)
                    onside_ahead_slowdown(RIGHT);
                else if (g_gear == RIGHT)
                    onside_ahead_slowdown(LEFT);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                g_gear = BACKWARD;
                bothside_goback_speedup();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'E': {  // 刹车
                printf("Break~\n");
                printf("=>Before Gear: %d\n", g_gear);
                g_gear = STOP;
                drive_break();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'D': {  // 右转
                printf("Turn Right~\n");
                printf("Current head: %d\n", CAR_HEAD);
                printf("=>Before Gear: %d\n", g_gear);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                if (CAR_HEAD == 1) {
                    if (g_gear != RIGHT) {
                        onside_ahead_slowdown(RIGHT);
                    }
                    onside_ahead_speedup(LEFT);
                } else {
                    if (g_gear != RIGHT) {
                        onside_goback_slowdown(RIGHT);
                    }
                    onside_goback_speedup(LEFT);
                }
                g_gear = RIGHT;
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            default: {
                printf("Restart.\n");
            }
        }
    }
    cleanup_pin();
}