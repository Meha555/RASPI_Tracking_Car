#include <pthread.h>
#include <signal.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <wiringPi.h>
#include "actuator/actuator.h"
#include "bcd/bcd.h"
#include "button/button.h"
#include "button/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "net/tcp_socket/raspi_tcp.h"
#include "ray/ray.h"
#include "sonar/hc_sr04.h"
#include "temperature/dht11.h"
#include "utils/thread_tools.h"

#define SPEED 40

#define COMMAND_LINE  // 如果开启了COMMAND_LINE 则注意是否与QT控制冲突

#define THREAD_NUM 6
pthread_t tid[THREAD_NUM];
struct termios tms_old, tms_new;  // 声明终端属性结构体

// 全局变量的定义
sem_t sem_keyboard;           // 键盘资源信号量
sem_t sem_sonar;              // 声呐资源信号量
pthread_mutex_t mutex_param;  // 电机参数锁
// pthread_mutex_t mutex_auto_keyctrl;   // 键盘控制锁
// pthread_mutex_t mutex_auto_tracking;  // 循迹控制锁

// int auto_keyctrl = 1, auto_tracking = 0;

void* tcpserver_thread(void* args);
void* button_thread(void* args);
void* temperature_thread(void* args);
void* sonar_thread(void* args);
void* keyboard_action_thread(void* args);
void* motor_thread(void* args);
void* tracking_thread(void* args);

// 捕获CTRL+C
static void catch_sigint(int sig) {
    printf("Kill all.\n");
    // 结束各个线程
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_cancel(tid[i]);
    }
    command_stop(0);   // motor
    command_reset(0);  // actuator
    command_shut(0);   // buzzer
    // TODO - 发送终止服务器2个线程的信号
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    // 清理引脚
    for (int pin = 1; pin <= 40; pin++)  // 树莓派3B板载40根引脚
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
    }

    signal(SIGINT, SIG_DFL);
    exit(0);  // 结束进程
}

// TCP服务器
void* tcpserver_thread(void* args) {
    printf("In tcpserver_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    setup_tcpserver(param);
}

// 按钮控制
void* button_thread(void* args) {
    printf("In button_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    initial_button();
    // button_action(catch_sigint);
    while (1) {
        // 调整最新的蜂鸣器电平
        buzzer_single_beep((int)param->buzzer_pin);
        // 调整最新的控制系统设置
        // if (param->keyctrl_switcher) {
        //     // if (pthread_kill(tid[3], 0) == 0) {
        //     //     printf("Disable auto tracking!\n");
        //     //     pthread_cancel(tid[3]);
        //     // }
        //     // printf("Activate keyboard control!\n");
        //     // pthread_create(&tid[1], NULL, keyboard_action_thread, param);
        //     // printf("Disable auto tracking!\n");
        //     auto_tracking = 0;
        //     // printf("Activate keyboard control!\n");
        //     auto_keyctrl = 1;
        // } else {
        //     // if (pthread_kill(tid[1], 0) == 0) {
        //     //     printf("Disable keyboard control!\n");
        //     //     pthread_cancel(tid[1]);
        //     // }
        //     // printf("Activate auto tracking!\n");
        //     // pthread_create(&tid[3], NULL, tracking_thread, param);
        //     // printf("Disable keyboard control!\n");
        //     auto_keyctrl = 0;
        //     // printf("Activate auto tracking!\n");
        //     auto_tracking = 1;
        // }
    }
}

// 测湿温度
void* temperature_thread(void* args) {
    printf("In temperature_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    unsigned char data[5] = {0};
    while (1) {
        switch (dht11_read_data(data)) {
            case 0:
                printf("Checksum Error! %d %d %d %d %d\n", data[0], data[1], data[2], data[3], data[4]);
                break;
            case 1:
                // printf("Humidity = %d.%d, Temperature = %d.%d\n", data[0], data[1], data[2], data[3]);
                bcd_display(data[0], data[1], data[2], data[3], 1);
                pthread_mutex_lock(&mutex_param);
                param->dht11_param.humidity = data[0] + data[1] / 10;
                param->dht11_param.temperature = data[2] + data[3] / 10;
                pthread_mutex_unlock(&mutex_param);
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
    struct TcpParam* param = (struct TcpParam*)args;
    inital_hr024();
    initial_actuator();
    while (1) {
        // 左转
        for (int i = -90; i < 90; i += 45) {
            if (param->servo_pin) {
                printf("Actuator spin left %d degree.\n", i);
                softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            }
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            // bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
            pthread_mutex_lock(&mutex_param);
            param->motor_param.dist = dist;  // 写入距离到参数地址
            if (param->motor_param.dist <= SPEED) {
                printf("Emergency Break!\n");
                param->motor_param.key_pressed = 'E';
                sem_post(&sem_keyboard);
                // DOUBLE_BEEP;
                buzzer_single_beep(1);
            } else
                buzzer_single_beep(0);
            pthread_mutex_unlock(&mutex_param);
            delay(500);
        }
        // 右转
        for (int i = 90; i > -90; i -= 45) {
            if (param->servo_pin) {
                printf("Actuator spin right %d degree.\n", i);
                softPwmWrite(SERVO_PIN, get_duty_cycle(i));
            }
            int dist = get_distance();
            if (dist >= 400)
                printf("Lose precision!\n");
            else
                printf("Dectecting dist: %d CM\n", dist);
            // bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
            pthread_mutex_lock(&mutex_param);
            param->motor_param.dist = dist;  // 写入距离到参数地址
            if (param->motor_param.dist <= SPEED) {
                printf("Emergency Break!\n");
                param->motor_param.key_pressed = 'E';
                sem_post(&sem_keyboard);
                // DOUBLE_BEEP;
                buzzer_single_beep(1);
            } else
                buzzer_single_beep(0);
            pthread_mutex_unlock(&mutex_param);
            delay(500);
        }
    }
}

// 键盘动作
void* keyboard_action_thread(void* args) {
    printf("In keyboard_action_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用标准输入的行缓冲和回显
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    unsigned char ch = 'E';
    while (1) {
#ifdef COMMAND_LINE
        while (!param->keyctrl_switcher) {
            pthread_yield(NULL);  // 主动放弃处理机
        }
#endif
        // 这段需要即时判断距离的代码不能和getchar()放在同一线程中，否则getchar()阻塞后将无法即时判断距离
        // if (param->dist <= 5) {
        //     pthread_mutex_lock(&mutex_param);
        //     printf("Emergency Break!\n");
        //     param->key_pressed = 'E';
        //     pthread_mutex_unlock(&mutex_param);
        // } else {
        ch = getchar();  // getchar()写锁外面，避免锁内阻塞
        pthread_mutex_lock(&mutex_param);
        param->motor_param.key_pressed = ch;
        pthread_mutex_unlock(&mutex_param);
        // }
        sem_post(&sem_keyboard);
    }
}

// 电机
void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    drive(param);
}

void* tracking_thread(void* args) {
    printf("In tracking_thread.\n");
    struct TcpParam* param = (struct TcpParam*)args;
    initial_tcrt5000();
    unsigned char ch = 'E';
    while (1) {
#ifdef COMMAND_LINE
        while (param->keyctrl_switcher) {
            pthread_yield(NULL);  // 主动放弃处理机
        }
#endif
        // printf("电位：%d %d %d %d\n", digitalRead(tracker.line_l), digitalRead(tracker.line_m1), digitalRead(tracker.line_m2), digitalRead(tracker.line_r));
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
        //     softPwmWrite(motor[LEFT].m1, SPEED);
        //     softPwmWrite(motor[RIGHT].m1, SPEED);
        // }
        // if (digitalRead(tracker.line_l) == LOW & digitalRead(tracker.line_m1) == LOW & digitalRead(tracker.line_m2) == LOW & digitalRead(tracker.line_r) == LOW) {
        //     // printf("掉头\n");
        //     softPwmWrite(motor[LEFT].m1, SPEED);
        //     softPwmWrite(motor[RIGHT].m2, SPEED);
        //     delay(SPEED);
        //     softPwmWrite(motor[LEFT].m1, 0);
        //     softPwmWrite(motor[RIGHT].m2, 0);
        // }

        // 中间都低电平，说明彻底没黑带，可以转弯
        if (digitalRead(tracker.line_m1) == LOW || digitalRead(tracker.line_m2) == LOW) {
            // 左侧高电平，右侧低电平，说明左侧有黑带，应该左转
            if ((digitalRead(tracker.line_l) == HIGH && digitalRead(tracker.line_r) == LOW) || (digitalRead(tracker.line_m1) == HIGH && digitalRead(tracker.line_m2) == LOW)) {
                printf("Turn Left permitted.\n");
                g_gear = LEFT;
                softPwmWrite(motor[RIGHT].m1, SPEED);
                softPwmWrite(motor[RIGHT].m2, 0);
                softPwmWrite(motor[LEFT].m1, 0);
                softPwmWrite(motor[LEFT].m2, SPEED);
                // ch = 'A';
                // pthread_mutex_lock(&mutex_param);
                // param->motor_param.key_pressed = ch;
                // pthread_mutex_unlock(&mutex_param);
                // sem_post(&sem_keyboard);
                while (digitalRead(tracker.line_m1) == LOW || digitalRead(tracker.line_m2) == LOW)
                    if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW)
                        delay(100);  // 彻底不在轨道上时，需要大转弯
                    else
                        ;  // 略微偏离轨道时，需要即时调整
            }
            // 右侧高电平，左侧低电平，说明右侧有黑带，应该右转
            else if (digitalRead(tracker.line_r) == HIGH && digitalRead(tracker.line_l) == LOW || (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == HIGH)) {
                printf("Turn Right permitted.\n");
                g_gear = RIGHT;
                softPwmWrite(motor[RIGHT].m1, 0);
                softPwmWrite(motor[RIGHT].m2, SPEED);
                softPwmWrite(motor[LEFT].m1, SPEED);
                softPwmWrite(motor[LEFT].m2, 0);
                // ch = 'D';
                // pthread_mutex_lock(&mutex_param);
                // param->motor_param.key_pressed = ch;
                // pthread_mutex_unlock(&mutex_param);
                // sem_post(&sem_keyboard);
                while (digitalRead(tracker.line_m1) == LOW || digitalRead(tracker.line_m2) == LOW) {
                    if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW)
                        delay(100);  // 彻底不在轨道上时，需要大转弯
                    else
                        ;  // 略微偏离轨道时，需要即时调整
                }
            }
            // 全部都为低电平，没有正下方的轨道了
            else if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW) {
                printf("No trail!\n");
                static int try_cnt = 1;
                if (try_cnt >= 0x3f3f3f3f) {
                    printf("Try failed.\n");
                    ch = 'E';
                    g_gear = STOP;
                    buzzer_single_beep(1);
                    pthread_mutex_lock(&mutex_param);
                    param->motor_param.key_pressed = ch;
                    pthread_mutex_unlock(&mutex_param);
                    sem_post(&sem_keyboard);
                } else {
                    printf("The %d times try\n", try_cnt);
                    if (g_gear == LEFT) {
                        CAR_HEAD = 0;  // 车头为后
                        printf("右后转弯\n");
                        // softPwmWrite(motor[RIGHT].m1, SPEED);
                        // softPwmWrite(motor[RIGHT].m2, 0);
                        // softPwmWrite(motor[LEFT].m1, 0);
                        // softPwmWrite(motor[LEFT].m2, SPEED);
                        ch = 'A';
                        pthread_mutex_lock(&mutex_param);
                        param->motor_param.key_pressed = ch;
                        pthread_mutex_unlock(&mutex_param);
                        sem_post(&sem_keyboard);
                        delay(650);  // 280
                    } else if (g_gear == RIGHT) {
                        CAR_HEAD = 0;  // 车头为后
                        printf("左后转弯\n");
                        // softPwmWrite(motor[RIGHT].m2, SPEED);
                        // softPwmWrite(motor[RIGHT].m1, 0);
                        // softPwmWrite(motor[LEFT].m2, 0);
                        // softPwmWrite(motor[LEFT].m1, SPEED);
                        ch = 'D';
                        pthread_mutex_lock(&mutex_param);
                        param->motor_param.key_pressed = ch;
                        pthread_mutex_unlock(&mutex_param);
                        sem_post(&sem_keyboard);
                        delay(650);  // 280
                    } else {
                        printf("后退\n");
                        // CAR_HEAD = 1;
                        // ch = 'S';
                        // pthread_mutex_lock(&mutex_param);
                        // param->motor_param.key_pressed = ch;
                        // pthread_mutex_unlock(&mutex_param);
                        // sem_post(&sem_keyboard);
                        // delay(500);
                        g_gear = BACKWARD;
                        softPwmWrite(motor[LEFT].m2, SPEED - 20);
                        softPwmWrite(motor[RIGHT].m2, SPEED - 20);
                        softPwmWrite(motor[LEFT].m1, 0);
                        softPwmWrite(motor[RIGHT].m1, 0);
                        delay(450);
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
                softPwmWrite(motor[LEFT].m1, SPEED - 10);
                softPwmWrite(motor[RIGHT].m1, SPEED - 10);
                softPwmWrite(motor[LEFT].m2, 0);
                softPwmWrite(motor[RIGHT].m2, 0);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    printf("--------------------------------------------------------------------\n");
    printf("Usage: ./main module_name(Leave blank to start all)\n");
    printf("modules: sonar, keyboard, motor, tracking, temperature, button, tcp\n");
    printf("Tips: motor must be activate before tracking or keyboard.\n");
    printf("--------------------------------------------------------------------\n");
    printf("Initializing...\n");
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    tm1637_init();
    initial_buzzer();
    signal(SIGINT, catch_sigint);
    tcgetattr(0, &tms_old);  // 获取旧的终端属性

    // 初始化互斥同步
    sem_init(&sem_sonar, 0, 0);
    sem_init(&sem_keyboard, 0, 0);
    pthread_mutex_init(&mutex_param, NULL);
    // pthread_mutex_init(&mutex_auto_keyctrl, NULL);
    // pthread_mutex_init(&mutex_auto_tracking, NULL);

    // 初始化参数
    // struct TcpParam* motor_param = (struct TcpParam*)malloc(sizeof(struct TcpParam));
    // motor_param->key_pressed = 'E';
    // motor_param->dist = 400;
    // motor_param->orient = AHEAD;
    struct TcpParam* tcp_param = (struct TcpParam*)malloc(sizeof(struct TcpParam));
    tcp_param->buzzer_pin = LOW;
    tcp_param->keyctrl_switcher = HIGH;
    tcp_param->servo_pin = LOW;
    tcp_param->motor_param.key_pressed = 'E';
    tcp_param->motor_param.dist = 400;
    tcp_param->motor_param.orient = AHEAD;
    tcp_param->dht11_param.temperature = 0;
    tcp_param->dht11_param.humidity = 0;

    // 创建线程，处理命令行参数
    printf("argc = %d,sizeof(*argv) = %d\n", argc, sizeof(*argv));
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {  // sizeof(*argv)
            printf("argv[%d] = %s\n", i, argv[i]);
            if (strcmp(argv[i], "sonar") == 0)
                pthread_create(&tid[i - 1], NULL, sonar_thread, tcp_param);
            else if (strcmp(argv[i], "keyboard") == 0)
                pthread_create(&tid[i - 1], NULL, keyboard_action_thread, tcp_param);
            else if (strcmp(argv[i], "motor") == 0)
                pthread_create(&tid[i - 1], NULL, motor_thread, tcp_param);
            else if (strcmp(argv[i], "tracking") == 0)
                pthread_create(&tid[i - 1], NULL, tracking_thread, tcp_param);
            else if (strcmp(argv[i], "temperature") == 0)
                pthread_create(&tid[i - 1], NULL, temperature_thread, tcp_param);
            else if (strcmp(argv[i], "button") == 0)
                pthread_create(&tid[i - 1], NULL, button_thread, tcp_param);
            else if (strcmp(argv[i], "tcp") == 0)
                pthread_create(&tid[i - 1], NULL, tcpserver_thread, tcp_param);
            else
                perror("Invalid Input argument!\n");
        }
    } else {
        pthread_create(&tid[0], NULL, sonar_thread, tcp_param);
        pthread_create(&tid[1], NULL, keyboard_action_thread, tcp_param);
        pthread_create(&tid[2], NULL, motor_thread, tcp_param);
        pthread_create(&tid[3], NULL, tracking_thread, tcp_param);
        pthread_create(&tid[4], NULL, temperature_thread, tcp_param);
        pthread_create(&tid[5], NULL, tcpserver_thread, tcp_param);
    }
    pthread_create(&tid[6], NULL, button_thread, tcp_param);

    // 阻塞主线程，等待子线程结束
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(tid[i], NULL);
    }
    printf("Exiting main process...\n");

    // 销毁互斥同步
    sem_destroy(&sem_sonar);
    sem_destroy(&sem_keyboard);
    pthread_mutex_destroy(&mutex_param);
    // pthread_mutex_destroy(&mutex_auto_keyctrl);
    // pthread_mutex_destroy(&mutex_auto_tracking);
    // 清理堆上变量
    free(tcp_param);
    return 0;
}