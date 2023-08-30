#include "motor.h"
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <wiringPi.h>

#define PWM_MAX 60    // PWM占空比
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
struct Motor {
    int m1;  // 正转引脚
    int m2;  // 反转引脚
} motor[2] = {
    {26, 27},
    {29, 28},
};

void command_stop(int sig) {
    for (int i = 0; i < 2; i++) {
        digitalWrite(motor[i].m1, LOW);
        digitalWrite(motor[i].m2, LOW);
        pinMode(motor[i].m1, INPUT);
        pinMode(motor[i].m2, INPUT);
    }
    signal(SIGINT, SIG_DFL);  // 重新注册回默认信号处理程序
    printf("Terminate.");
}
void inital_drive() {
    printf("Initializing...\n");
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    for (int i = 0; i < 2; i++) {
        pinMode(motor[i].m1, OUTPUT);
        pinMode(motor[i].m2, OUTPUT);
        softPwmCreate(motor[i].m1, 0, PWM_MAX);  // 创建一个软件控制的 PWM 管脚【将m1设置为PWM控制】
        softPwmCreate(motor[i].m2, 0, PWM_MAX);  // 创建一个软件控制的 PWM 管脚【将m2设置为PWM控制】
    }
    signal(SIGINT, command_stop);  // 注册我们的CTRL+C停止点击的信号处理程序
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
void drive(double (*fp1)(void), void (*fp2)(int, int, int, int, int)) {
    inital_drive();
    int opt;
    printf("Ready, waitting for command...\n");
    struct termios tms_old, tms_new;  // 声明终端属性结构体
    tcgetattr(0, &tms_old);           // 获取旧的终端属性
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    int dist = 0;
    while (1) {
        dist = (*fp1)();
        (*fp2)(0, dist / 100, dist % 100 / 10, dist % 10, 0);
        printf("完成测距\n");
        printf("Dectecting dist: %d CM\n", dist);
        if (dist <= 5) {
            printf("Emergency Break~\n");
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            g_gear = STOP;
            drive_break();
            delay(500);
        }
        unsigned char ch = getchar();  // 读取用户输入的字符
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
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    cleanup_pin();
}