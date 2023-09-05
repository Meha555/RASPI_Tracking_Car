#ifndef RASPI_TCP_H
#define RASPI_TCP_H

#include "tcp_header.h"

#define PORT 8888  // TCP采用的端口号

struct TcpParam {
    struct {
        unsigned char key_pressed;
        int dist;
        enum Orientation { AHEAD,
                           TURN_LEFT,
                           TURN_RIGHT,
        } orient;
    } motor_param;
    struct {
        double temperature;
        double humidity;
    } dht11_param;
    int buzzer_pin;
};

extern int listenfd, connfd;                     // listenfd是服务器套接字，用于监听；connfd是存储客户端的套接字，用于与连接上的客户端通信
extern struct sockaddr_in serveraddr, peeraddr;  // serveraddr是服务器自身socket地址，peeraddr是对端socket地址
extern socklen_t len;
extern char buf[BUFF_SIZE];
extern ssize_t n;

extern void setup_tcpserver(struct TcpParam* param);  // 建立TCP服务器
extern void send_tcp_segment();                       // 发送TCP报文段
extern void recv_tcp_segment();                       // 接收TCP报文段

#endif  // RASPI_TCP_H