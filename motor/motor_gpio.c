#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <wiringPi.h>
#define PWM_MAX 80
#define PWM_DELTA 5
struct Motor {
    int m1;  // 正转引脚
    int m2;  // 反转引脚
};
enum Direct {
    RIGHT,
    LEFT,
    FORWARD,
    BACKWARD
};
struct Motor motor[2] = {
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
        digitalWrite(motor[i].m1, LOW);
        digitalWrite(motor[i].m2, LOW);
    }
    signal(SIGINT, command_stop);  // 注册我们的CTRL+C停止点击的信号处理程序
}
void cleanup_pin() {
    for (int i = 0; i < 2; i++) {
        pinMode(motor[i].m1, INPUT);
        pinMode(motor[i].m2, INPUT);
    }
}
/**
 * @description: 匀速前进
 * @return {void}
 */
void drive_forward() {
    digitalWrite(motor[LEFT].m1, HIGH);
    digitalWrite(motor[LEFT].m2, LOW);
    digitalWrite(motor[RIGHT].m1, HIGH);
    digitalWrite(motor[RIGHT].m2, LOW);
    // while(1);
}
/**
 * @description: 匀速后退
 * @return {void}
 */
void drive_backward() {
    digitalWrite(motor[LEFT].m1, LOW);
    digitalWrite(motor[LEFT].m2, HIGH);
    digitalWrite(motor[RIGHT].m1, LOW);
    digitalWrite(motor[RIGHT].m2, HIGH);
    // while(1);
}
/**
 * @description: 电机转向
 * @param {enum Direct} 方向 - 左转or右转
 * @return {void}
 */
void drive_turn(enum Direct direct) {
    digitalWrite(motor[direct].m1, LOW);
    digitalWrite(motor[direct].m2, LOW);
    // while(1);
}
int main() {
    inital_drive();
    int opt;
    printf("Ready, waitting for command...\n");
    struct termios tms_old, tms_new;  // 声明终端属性结构体
    tcgetattr(0, &tms_old);           // 获取旧的终端属性
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    while (1) {
        unsigned char ch = getchar();  // 读取用户输入的字符
        switch (ch) {
            case 'W': {  // 前进
                printf("Forward~\n");
                drive_forward();
                break;
            }
            case 'A': {  // 左转
                printf("Turn Left~\n");
                drive_turn(LEFT);
                break;
            }
            case 'S': {  // 后退
                printf("Backward~\n");
                drive_backward();
                break;
            }
            case 'D': {  // 右转
                printf("Turn Right~\n");
                drive_turn(RIGHT);
                break;
            }
            default: {
                printf("Restart.\n");
            }
        }
    }
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    cleanup_pin();
    return 0;
}