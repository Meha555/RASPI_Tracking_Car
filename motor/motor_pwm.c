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

// int g_current_speed = 0;
int curr_speed[2] = {0};

enum Direct {
    RIGHT,
    LEFT,
    FORWARD,
    BACKWARD,
    STOP
} g_gear = STOP;  // 当前档位
// enum Gear {
//     AHEAD,
//     GOBACK,
//     STOP
// }
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
void cleanup_pin() {
    for (int i = 0; i < 2; i++) {
        pinMode(motor[i].m1, INPUT);
        pinMode(motor[i].m2, INPUT);
    }
}
/**
 * @description: 电机转向
 * @param {enum Direct} 方向 - 左转or右转
 * @return {void}
 */
void drive_turn(enum Direct direct) {
    int pin_slow = (direct == LEFT) ? motor[LEFT].m1 : motor[RIGHT].m1;
    int pin_fast = (direct == LEFT) ? motor[RIGHT].m1 : motor[LEFT].m1;
    for (; curr_speed[~direct] < PWM_MAX; curr_speed[~direct] += (PWM_DELTA << 1)) {
        softPwmWrite(pin_fast, curr_speed[~direct]);
        printf("drive_turn fast: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
        delay(200);
    }
    for (; curr_speed[direct] > 0; curr_speed[direct] -= (PWM_DELTA << 1)) {
        softPwmWrite(pin_slow, curr_speed[direct]);
        printf("drive_turn slow: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
        delay(200);
    }
}
/**
 * @description: PWM加速 - 每次g_current_speed
 * @param {enum Direct} 方向 - 前进or后退
 * @return {void}
 */
void drive_speed_up() {
    int pin_left = (g_gear == FORWARD) ? motor[LEFT].m1 : motor[LEFT].m2;
    int pin_right = (g_gear == FORWARD) ? motor[RIGHT].m1 : motor[RIGHT].m2;
    for (; curr_speed[LEFT] < PWM_MAX && curr_speed[RIGHT] < PWM_MAX; curr_speed[LEFT] += PWM_DELTA, curr_speed[RIGHT] += PWM_DELTA) {
        softPwmWrite(pin_left, curr_speed[LEFT]);
        softPwmWrite(pin_right, curr_speed[RIGHT]);
        delay(500);
    }
}
/**
 * @description: PWM减速 - 每次g_current_speed
 * @param {enum Direct} 方向 - 前进or后退
 * @return {void}
 */
void drive_speed_down() {
    int pin_left = (g_gear == FORWARD) ? motor[LEFT].m1 : motor[LEFT].m2;
    int pin_right = (g_gear == FORWARD) ? motor[RIGHT].m1 : motor[RIGHT].m2;
    for (; curr_speed[LEFT] > 0 && curr_speed[RIGHT] > 0; curr_speed[LEFT] -= PWM_DELTA, curr_speed[RIGHT] -= PWM_DELTA) {
        softPwmWrite(pin_left, curr_speed[LEFT]);
        softPwmWrite(pin_right, curr_speed[RIGHT]);
        delay(500);
    }
}
/**
 * @description: 电机刹车
 * @return {void}
 */
void drive_break() {
    for (int i = 0; i < 2; i++) {
        softPwmWrite(motor[i].m1, 0);
        softPwmWrite(motor[i].m2, 0);
        curr_speed[i] = 0;
    }
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
                printf("=>Before Gear: %d\n", g_gear);
                if (g_gear == BACKWARD)
                    drive_speed_down();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                g_gear = FORWARD;
                drive_speed_up();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'A': {  // 左转
                printf("Turn Left~\n");
                printf("=>Before Gear: %d\n", g_gear);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                drive_turn(LEFT);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                break;
            }
            case 'S': {  // 后退
                printf("Backward~\n");
                printf("=>Before Gear: %d\n", g_gear);
                if (g_gear == FORWARD)
                    drive_speed_down();
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                g_gear = BACKWARD;
                drive_speed_up();
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
                printf("=>Before Gear: %d\n", g_gear);
                printf("Current speed: %d %d\n", curr_speed[LEFT], curr_speed[RIGHT]);
                drive_turn(RIGHT);
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
    return 0;
}