#ifndef RASPI_TCP_H
#define RASPI_TCP_H

#include <pthread.h>
#include "../../motor/motor.h"
#include "tcp_header.h"

#define PORT 1200  // TCP采用的端口号

// 24 bytes
struct TcpParam {
    unsigned char buzzer_pin;        // 1 byte
    unsigned char keyctrl_switcher;  // 1 byte
    unsigned char servo_pin;         // 1 byte
    struct {
        unsigned char key_pressed;  // 1 byte
        int dist;                   // 4 bytes
        enum Orientation { AHEAD,
                           TURN_LEFT,
                           TURN_RIGHT,
        } orient;   // 4 bytes
    } motor_param;  // 12 bytes
    struct {
        float temperature;
        float humidity;
    } dht11_param;  // 8 bytes
};

#define PRINT_TCP_PARAM(param)                                         \
    do {                                                               \
        printf("**buzzer pin: %d\n", param->buzzer_pin);               \
        printf("**keyctrl: %d\n", param->keyctrl_switcher);            \
        printf("**soar pin: %c\n", param->servo_pin);                  \
        printf("**key_pressed: %c\n", param->motor_param.key_pressed); \
        printf("**dist: %d\n", param->motor_param.dist);               \
        printf("**orient: ");                                          \
        if (param->motor_param.orient == 0)                            \
            printf("AHEAD\n");                                         \
        else if (param->motor_param.orient == 1)                       \
            printf("TURN_LEFT\n");                                     \
        else if (param->motor_param.orient == 2)                       \
            printf("TURN_RIGHT\n");                                    \
        printf("**temperature: %f\n", param->dht11_param.temperature); \
        printf("**humidity: %f\n", param->dht11_param.humidity);       \
    } while (0);

extern int listenfd, connfd;                     // listenfd是服务器套接字，用于监听；connfd是存储客户端的套接字，用于与连接上的客户端通信
extern struct sockaddr_in serveraddr, peeraddr;  // serveraddr是服务器自身socket地址，peeraddr是对端socket地址
extern socklen_t len;
extern char buf[BUFF_SIZE], buf_recv[BUFF_SIZE];
extern ssize_t n;
// extern sem_t sem_keyboard;  // 键盘资源信号量

extern void setup_tcpserver(struct TcpParam* param);  // 建立TCP服务器

#endif  // RASPI_TCP_H