#include "actuator.h"
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

// [-90,0] -> [0.5ms,1.5ms]
// (0,90] -> [1.5ms,2.5ms]
int get_duty_cycle(int angle) {
    return angle / 9 + 15;  // 可以发现角度和占空比的对应是线性函数
}

void command_reset(int sig) {
    printf("Actuator reset to 0 degree.\n");
    softPwmWrite(SERVO_PIN, get_duty_cycle(0));  // 回正（0度）
    delay(500);                                  // 回正需要时间
    pinMode(SERVO_PIN, INPUT);
    signal(SIGINT, SIG_DFL);
}

void initial_actuator() {
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    pinMode(SERVO_PIN, INPUT);
    softPwmCreate(SERVO_PIN, 0, 200);  // 创建softPwm对象
    signal(SIGINT, command_reset);
}

void control_spin() {
    while (1) {
        // 左转
        for (int i = -90; i <= 90; i++) {
            // printf("Actuator spin left %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            delay(15);
        }
        // 右转
        for (int i = 90; i >= -90; i--) {
            // printf("Actuator spin right %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            delay(15);
        }
    }
}

// int main() {
//     initial_actuator();
//     control_spin();
//     return 0;
// }
