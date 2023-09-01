#ifndef MOTOR_H
#define MOTOR_H

#include <pthread.h>
#include <semaphore.h>

struct MotorParam {
    unsigned char key_pressed;
    int dist;
    enum Orientation { AHEAD,
                       TURN_LEFT,
                       TURN_RIGHT } orient;
};

extern struct Motor {
    int m1;  // 正转引脚
    int m2;  // 反转引脚
} motor[2];

extern sem_t sem_keyboard;           // 键盘资源信号量
extern sem_t sem_sonar;              // 声呐资源信号量
extern pthread_mutex_t mutex_param;  // 电机参数锁

extern void inital_drive();
extern void drive(struct MotorParam* param);
extern void command_stop(int);
// void bothside_ahead_speedup();
// void bothside_ahead_slowdown();
// void bothside_goback_speedup();
// void bothside_goback_slowdown();
// void onside_ahead_speedup(enum Direct direct);
// void onside_goback_speedup(enum Direct direct);
// void onside_ahead_slowdown(enum Direct direct);
// void onside_goback_slowdown(enum Direct direct);
// void drive_break();

#endif  // MOTOR_H