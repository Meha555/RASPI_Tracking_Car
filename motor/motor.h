#ifndef MOTOR_H
#define MOTOR_H

#include <pthread.h>
#include <semaphore.h>
#include "../net/tcp_socket/raspi_tcp.h"

#define PRINT_GEAR                          \
    do {                                    \
        if (g_gear == 0)                    \
            printf("****GEAR: LEFT\n");     \
        else if (g_gear == 1)               \
            printf("****GEAR: RIGHT\n");    \
        else if (g_gear == 2)               \
            printf("****GEAR: FORWARD\n");  \
        else if (g_gear == 3)               \
            printf("****GEAR: BACKWARD\n"); \
        else if (g_gear == 4)               \
            printf("****GEAR: STOP\n");     \
    } while (0);

enum Direct {
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    STOP
};

extern struct Motor {
    int m1;  // 正转引脚
    int m2;  // 反转引脚
} motor[2];

extern sem_t sem_keyboard;           // 键盘资源信号量
extern sem_t sem_sonar;              // 声呐资源信号量
extern pthread_mutex_t mutex_param;  // 电机参数锁

extern void inital_drive();
extern void drive(struct TcpParam* param);
extern void command_stop(int);

extern void bothside_ahead_speedup();
extern void bothside_ahead_slowdown();
extern void bothside_goback_speedup();
extern void bothside_goback_slowdown();
extern void onside_ahead_speedup(enum Direct direct);
extern void onside_goback_speedup(enum Direct direct);
extern void onside_ahead_slowdown(enum Direct direct);
extern void onside_goback_slowdown(enum Direct direct);
extern void drive_break();

extern int g_gear;    // 当前档位(这里貌似不能写enum Direct枚举，会在motor.c中认不出g_gear是枚举类型)
extern int CAR_HEAD;  // 车头方向

#endif  // MOTOR_H