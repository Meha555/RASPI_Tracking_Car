#include <pthread.h>
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <unistd.h>
#include <wiringPi.h>
#include "actuator/actuator.h"
#include "bcd/bcd.h"
#include "button/button.h"
#include "button/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "ray/ray.h"
#include "sonar/hc_sr04.h"
#include "temperature/dht11.h"
#include "utils/thread_tools.h"

#define THREAD_NUM 6
pthread_t tid[THREAD_NUM];
struct termios tms_old, tms_new;  // 声明终端属性结构体

// 全局变量的定义
sem_t sem_keyboard;           // 键盘资源信号量
sem_t sem_sonar;              // 声呐资源信号量
pthread_mutex_t mutex_param;  // 电机参数锁

// 捕获CTRL+C
static void catch_sigint(int sig) {
    printf("Kill all.\n");
    // 结束各个线程
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_cancel(tid[i]);
    }
    command_stop(0);                  // motor
    command_reset(0);                 // actuator
    command_shut(0);                  // buzzer
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    signal(SIGINT, SIG_DFL);
    exit(0);  // 结束进程
}

// 按钮控制
void* button_thread(void* args) {
    printf("In button_thread.\n");
    initial_button();
    button_action(catch_sigint);
}

// 测湿温度
void* temperature_thread(void* args) {
    printf("In temperature_thread.\n");
    unsigned char data[5] = {0};
    while (1) {
        switch (dht11_read_data(data)) {
            case 0:
                printf("Checksum Error! %d %d %d %d %d\n", data[0], data[1], data[2], data[3], data[4]);
                break;
            case 1:
                printf("Humidity = %d.%d, Temperature = %d.%d\n", data[0], data[1], data[2], data[3]);
                bcd_display(data[0], data[1], data[2], data[3], 1);
                break;
            case 2:
                printf("Time Out!\n");
                break;
            case 3:
                printf("DHT11 no response!\n");
                break;
        }
        delay(1000);
    }
}

// 超声波+舵机+数码管
void* sonar_thread(void* args) {
    printf("In sonar_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    tm1637_init();
    inital_hr024();
    initial_actuator();
    initial_buzzer();
    int flag = 1;
    while (1) {
        // 左转
        for (int i = -90; i < 90; i += 45) {
            // printf("Actuator spin left %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
            pthread_mutex_lock(&mutex_param);
            param->dist = dist;  // 写入距离到参数地址
            if (param->dist <= 30) {
                printf("Emergency Break!\n");
                param->key_pressed = 'E';
                sem_post(&sem_keyboard);
                // DOUBLE_BEEP;
                buzzer_beep(1);
            } else
                buzzer_beep(0);
            pthread_mutex_unlock(&mutex_param);
            delay(500);
        }
        // 右转
        for (int i = 90; i > -90; i -= 45) {
            // printf("Actuator spin right %d degree.\n", i);
            softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
            pthread_mutex_lock(&mutex_param);
            param->dist = dist;  // 写入距离到参数地址
            if (param->dist <= 30) {
                printf("Emergency Break!\n");
                param->key_pressed = 'E';
                sem_post(&sem_keyboard);
                // DOUBLE_BEEP;
                buzzer_beep(1);
            } else
                buzzer_beep(0);
            pthread_mutex_unlock(&mutex_param);
            delay(500);
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
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    unsigned char ch = 'E';
    while (1) {
        // 这段需要即时判断距离的代码不能和getchar()放在同一线程中，否则getchar()阻塞后将无法即时判断距离
        // if (param->dist <= 5) {
        //     pthread_mutex_lock(&mutex_param);
        //     printf("Emergency Break!\n");
        //     param->key_pressed = 'E';
        //     pthread_mutex_unlock(&mutex_param);
        // } else {
        ch = getchar();  // getchar()写锁外面，避免锁内阻塞
        pthread_mutex_lock(&mutex_param);
        param->key_pressed = ch;
        pthread_mutex_unlock(&mutex_param);
        // }
        sem_post(&sem_keyboard);
    }
}

// 电机
void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    drive(param);
}

void* tracking_thread(void* args) {
    printf("In actuator_thread.\n");
    struct MotorParam* param = (struct MotorParam*)args;
    initial_tcrt5000();
    unsigned char ch = 'E';
    while (1) {
        printf("电位：%d %d %d %d\n", digitalRead(tracker.line_l), digitalRead(tracker.line_m1), digitalRead(tracker.line_m2), digitalRead(tracker.line_r));
        // while ((digitalRead(tracker.line_l) == HIGH & digitalRead(tracker.line_m1) == LOW & digitalRead(tracker.line_m2) == LOW & digitalRead(tracker.line_r) == LOW)) {
        //     printf("左转\n");
        //     softPwmWrite(motor[LEFT].m2, 40);
        //     softPwmWrite(motor[RIGHT].m1, 40);
        //     while (!(digitalRead(tracker.line_m1) == HIGH & digitalRead(tracker.line_m2) == HIGH))
        //         ;
        //     // delay(25);
        //     softPwmWrite(motor[LEFT].m2, 0);
        //     softPwmWrite(motor[RIGHT].m1, 0);
        // }
        // while (digitalRead(tracker.line_l) == LOW & digitalRead(tracker.line_m1) == LOW & digitalRead(tracker.line_m2) == LOW & digitalRead(tracker.line_r) == HIGH) {
        //     printf("右转\n");
        //     softPwmWrite(motor[RIGHT].m2, 40);
        //     softPwmWrite(motor[LEFT].m1, 40);
        //     while (!(digitalRead(tracker.line_m1) == HIGH & digitalRead(tracker.line_m2) == HIGH))
        //         ;
        //     // delay(25);
        //     softPwmWrite(motor[RIGHT].m2, 0);
        //     softPwmWrite(motor[LEFT].m1, 0);
        // }
        // while (digitalRead(tracker.line_m1) == HIGH | digitalRead(tracker.line_m2) == HIGH) {
        //     // printf("直行\n");
        //     softPwmWrite(motor[LEFT].m1, 30);
        //     softPwmWrite(motor[RIGHT].m1, 30);
        // }
        // if (digitalRead(tracker.line_l) == LOW & digitalRead(tracker.line_m1) == LOW & digitalRead(tracker.line_m2) == LOW & digitalRead(tracker.line_r) == LOW) {
        //     // printf("掉头\n");
        //     softPwmWrite(motor[LEFT].m1, 30);
        //     softPwmWrite(motor[RIGHT].m2, 30);
        //     delay(30);
        //     softPwmWrite(motor[LEFT].m1, 0);
        //     softPwmWrite(motor[RIGHT].m2, 0);
        // }

        // 中间都低电平，说明没黑带，可以转弯
        if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW) {
            // 左侧高电平，右侧低电平，说明左侧碰到黑带，应该左转
            if (digitalRead(tracker.line_l) == HIGH && digitalRead(tracker.line_r) == LOW) {
                printf("Turn Left permitted.\n");
                g_gear = LEFT;

                softPwmWrite(motor[RIGHT].m1, 30);
                softPwmWrite(motor[RIGHT].m2, 0);
                softPwmWrite(motor[LEFT].m1, 0);
                softPwmWrite(motor[LEFT].m2, 30);
                // ch = 'A';
                // pthread_mutex_lock(&mutex_param);
                // param->key_pressed = ch;
                // pthread_mutex_unlock(&mutex_param);
                // sem_post(&sem_keyboard);
                // delay(80);
                while (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW)
                    ;
            }
            // 右侧高电平，左侧低电平，说明右侧碰到黑带，应该右转
            else if (digitalRead(tracker.line_r) == HIGH && digitalRead(tracker.line_l) == LOW) {
                printf("Turn Right permitted.\n");
                g_gear = RIGHT;
                softPwmWrite(motor[RIGHT].m1, 0);
                softPwmWrite(motor[RIGHT].m2, 30);
                softPwmWrite(motor[LEFT].m1, 30);
                softPwmWrite(motor[LEFT].m2, 0);
                // ch = 'D';
                // pthread_mutex_lock(&mutex_param);
                // param->key_pressed = ch;
                // pthread_mutex_unlock(&mutex_param);
                // sem_post(&sem_keyboard);
                // delay(80);
                while (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW)
                    ;
            }
            // 全部都为低电平，没有正下方的轨道了
            else if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW) {
                printf("No trail!\n");
                static try_cnt = 1;
                if (try_cnt >= 5) {
                    printf("Try failed.\n");
                    ch = 'E';
                    buzzer_beep(1);
                    pthread_mutex_lock(&mutex_param);
                    param->key_pressed = ch;
                    pthread_mutex_unlock(&mutex_param);
                    sem_post(&sem_keyboard);
                } else {
                    printf("On the %d times try\n", try_cnt);
                    CAR_HEAD = 0;  // 车头为后

                    if (g_gear == LEFT) {
                        printf("右后转弯\n");
                        softPwmWrite(motor[RIGHT].m1, 30);
                        softPwmWrite(motor[RIGHT].m2, 0);
                        softPwmWrite(motor[LEFT].m1, 0);
                        softPwmWrite(motor[LEFT].m2, 30);
                        // ch = 'A';
                        // pthread_mutex_lock(&mutex_param);
                        // param->key_pressed = ch;
                        // pthread_mutex_unlock(&mutex_param);
                        // sem_post(&sem_keyboard);
                        delay(280);
                    } else if (g_gear == RIGHT) {
                        printf("左后转弯\n");
                        softPwmWrite(motor[RIGHT].m2, 30);
                        softPwmWrite(motor[RIGHT].m1, 0);
                        softPwmWrite(motor[LEFT].m2, 0);
                        softPwmWrite(motor[LEFT].m1, 30);
                        // ch = 'D';
                        // pthread_mutex_lock(&mutex_param);
                        // param->key_pressed = ch;
                        // pthread_mutex_unlock(&mutex_param);
                        // sem_post(&sem_keyboard);
                        delay(280);
                    } else {
                        printf("后退\n");
                        ch = 'S';
                        pthread_mutex_lock(&mutex_param);
                        param->key_pressed = ch;
                        pthread_mutex_unlock(&mutex_param);
                        sem_post(&sem_keyboard);
                        delay(500);
                        g_gear = FORWARD;
                    }
                    try_cnt++;
                    CAR_HEAD = 1;
                }
            }
        }
        // 中间至少有一个为高电平，说明有黑带，可以直行
        else if (digitalRead(tracker.line_m1) == HIGH || digitalRead(tracker.line_m2) == HIGH) {
            // 左右两侧都是低电平，说明可以直走
            if (digitalRead(tracker.line_l) == LOW && digitalRead(tracker.line_r) == LOW) {
                printf("Forward permitted.\n");
                g_gear = FORWARD;
                // ch = 'W';
                softPwmWrite(motor[LEFT].m1, 30);
                softPwmWrite(motor[RIGHT].m1, 30);
                softPwmWrite(motor[LEFT].m2, 0);
                softPwmWrite(motor[RIGHT].m2, 0);
                // delay(50);
            }
        }
        //     delay(100);
    }

    // pthread_mutex_lock(&mutex_param);
    // param.orient =
    // printf("Submit key_pressed %c.\n", param.key_pressed);
    // pthread_mutex_unlock(&mutex_param);
}

int main(int argc, char* argv[]) {
    printf("Initializing...\n");
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    signal(SIGINT, catch_sigint);
    tcgetattr(0, &tms_old);  // 获取旧的终端属性

    // 初始化互斥同步
    sem_init(&sem_sonar, 0, 0);
    sem_init(&sem_keyboard, 0, 0);
    pthread_mutex_init(&mutex_param, NULL);

    // 初始化参数
    struct MotorParam* motor_param = (struct MotorParam*)malloc(sizeof(struct MotorParam));
    motor_param->key_pressed = 'E';
    motor_param->dist = 400;
    motor_param->orient = AHEAD;

    // 创建线程，处理命令行参数
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {  // sizeof(*argv)
            // printf("argv[%d] = %s\n", i, argv[i]);
            if (argv[i - 1] == "sonar")
                pthread_create(&tid[i], NULL, sonar_thread, motor_param);
            if (argv[i - 1] == "keyboard")
                pthread_create(&tid[i], NULL, keyboard_action_thread, motor_param);
            if (argv[i - 1] == "motor")
                pthread_create(&tid[i], NULL, motor_thread, motor_param);
            if (argv[i - 1] == "tracking")
                pthread_create(&tid[i], NULL, tracking_thread, motor_param);
            if (argv[i - 1] == "temperature")
                pthread_create(&tid[i], NULL, temperature_thread, motor_param);
            if (argv[i - 1] == "button")
                pthread_create(&tid[i], NULL, button_thread, motor_param);
        }
    } else {
        pthread_create(&tid[0], NULL, sonar_thread, motor_param);
        pthread_create(&tid[1], NULL, keyboard_action_thread, motor_param);
        pthread_create(&tid[2], NULL, motor_thread, motor_param);
        // pthread_create(&tid[3], NULL, tracking_thread, motor_param);
        pthread_create(&tid[4], NULL, temperature_thread, NULL);
        pthread_create(&tid[5], NULL, button_thread, NULL);
    }

    // 阻塞主线程，等待子线程结束
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