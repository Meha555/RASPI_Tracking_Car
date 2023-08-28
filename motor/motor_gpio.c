#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <wiringPi.h>
#define PWM_MAX 100
#define PWM_DELTA 10
struct Motor {
    int m1;  // 正转引脚
    int m2;  // 反转引脚
};
enum Direct {
    RIGHT,
    LEFT
};
struct Motor motor[2] = {
    {26, 27},
    {28, 29},
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
void drive_forward_backward(struct Motor motor) {
    while (1) {
        printf("Forward~\n");
        digitalWrite(motor.m1, HIGH);
        digitalWrite(motor.m2, LOW);
        delay(10000);
        printf("Backward~\n");
        digitalWrite(motor.m1, LOW);
        digitalWrite(motor.m2, HIGH);
        delay(10000);
    }
}
void drive_pwm_oneside(struct Motor motor, enum Direct direct) {
    int pin = 0;
    if (direct == LEFT)
        pin = motor.m1;
    else
        pin = motor.m2;
    softPwmCreate(motor.m1, 0, PWM_MAX);  // 创建一个软件控制的 PWM 管脚【将m1设置为PWM控制】
    softPwmCreate(motor.m2, 0, PWM_MAX);  // 创建一个软件控制的 PWM 管脚【将m2设置为PWM控制】
    while (1) {
        // 正转加速
        printf("Speed Up~\n");
        for (int i; i < PWM_MAX; i += PWM_DELTA) {
            softPwmWrite(pin, i);  // 更新PWM的值
            delay(10);
        }
        delay(5000);
        // 正转减速
        printf("Speed Down~\n");
        for (int i = PWM_MAX; i > 0; i -= PWM_DELTA) {
            softPwmWrite(pin, i);  // 更新PWM的值
            delay(10);
        }
    }
}
void drive_pwm_speedchange(struct Motor motor, enum Direct direct) {
    // 正转加速
    printf("Forward Speed Up~\n");
    for (int i = 0; i < PWM_MAX; i += PWM_DELTA) {
        softPwmWrite(motor.m1, i);
        delay(10);
    }
    sleep(5);
    // 正转减速
    printf("Forward Speed Down~\n");
    for (int i = PWM_MAX; i > 0; i -= PWM_DELTA) {
        softPwmWrite(motor.m1, i);
        delay(10);
    }
    sleep(5);
    // 反转减速
    printf("Backward Speed Up~\n");
    for (int i = PWM_MAX; i > 0; i -= PWM_DELTA) {
        softPwmWrite(motor.m2, i);
        delay(10);
    }
    sleep(5);
    // 反转减速
    printf("Backward Speed Down~\n");
    for (int i = PWM_MAX; i > 0; i -= PWM_DELTA) {
        softPwmWrite(motor.m2, i);
        delay(10);
    }
    sleep(5);
}
int main() {
    inital_drive();
    int opt;
start:
    printf("Drive command: ");
    scanf("%d", &opt);
    switch (opt) {
        case 1: {  // 10s前进10s后退
            drive_forward_backward(motor[RIGHT]);
            break;
        }
        case 2: {
            drive_pwm_oneside(motor[RIGHT], RIGHT);
            break;
        }
        case 3: {
            drive_pwm_speedchange(motor[RIGHT], RIGHT);
            break;
        }
        default: {
            printf("Restart.");
            goto start;
        }
    }
    // printf("Ready, waitting for command...\n");
    // struct termios tms_old, tms_new;  // 声明终端属性结构体
    // tcgetattr(0, &tms_old);           // 获取旧的终端属性
    // // 设置新的终端属性
    // tms_new = tms_old;                    // 复制终端属性
    // tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    // tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    // while (1) {
    //     unsigned char ch = getchar();  // 读取用户输入的字符
    //     printf("按下的是: %c\n", ch);
    // }
    // tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    return 0;
}