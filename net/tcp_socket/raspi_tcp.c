#include "raspi_tcp.h"

int listenfd, connfd;
struct sockaddr_in serveraddr, peeraddr;
socklen_t len;
char buf_send[BUFF_SIZE], buf_recv[BUFF_SIZE];
static pthread_t tid[2];  // 一个接收线程，一个发送线程
static int retval[2];     // 线程返回值，用于决定是否关闭服务器

// 发送TCP报文段
void* send_tcp_segment(struct TcpParam* args) {
    struct TcpParam* param = (struct TcpParam*)args;
    while (1) {
        // 服务器往客户端发送数据
        memset(buf_send, 0, sizeof(buf_send));
        printf("Sending...\n");
        ssize_t n = sizeof(struct TcpParam);
        memcpy(buf_send, param, n);
        // PRINT_TCP_PARAM(param);
        //  向客户端套接字中发送数据
        send(connfd, buf_send, n, 0);
        sleep(1);
    }
}

// 接收TCP报文段
void* recv_tcp_segment(struct TcpParam* args) {
    struct TcpParam* param = (struct TcpParam*)args;
    while (1) {
        // 服务器接收客户端发送的数据
        memset(buf_recv, 0, sizeof(buf_recv));
        printf("Receiving...\n");
        ssize_t n = recv(connfd, buf_recv, BUFF_SIZE, 0);
        if (n <= 0) {
            pthread_exit(0);
        } else
            printf("接收到的长度: %d\n", n);
        if (strcmp(buf_recv, "close") == 0) {
            printf("Server shutting down~\n");
            pthread_cancel(tid[0]);  // 终止发送线程
            memset(buf_send, 0, sizeof(buf_send));
            send(connfd, "ack", 3, 0);
            continue;
        }
        struct TcpParam* recv_param = (struct TcpParam*)buf_recv;
        // PRINT_TCP_PARAM(recv_param);
        pthread_mutex_lock(&mutex_param);
        param->buzzer_pin = recv_param->buzzer_pin;
        param->keyctrl_switcher = recv_param->keyctrl_switcher;
        param->motor_param.key_pressed = recv_param->motor_param.key_pressed;
        // param->motor_param.dist = recv_param->motor_param.dist;
        // param->motor_param.orient = recv_param->motor_param.orient;
        // param->dht11_param.temperature = recv_param->dht11_param.temperature;
        // param->dht11_param.humidity = recv_param->dht11_param.humidity;
        pthread_mutex_unlock(&mutex_param);
        sem_post(&sem_keyboard);
    }
}

void setup_tcpserver(struct TcpParam* param) {
    // 创建监听客户端的套接字 - 确定套接字的各种属性（地址类型、协议族）
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {  // 使用IPv4地址 面向字节流和连接的协议 指定是TCP协议（也可以自动推导）
        perror("Socket Create Failure!");
        exit(-1);
    }
    // 将套接字与特定的IP地址和端口绑定起来，使得来自该IP地址并经过该端口的数据交由该套接字处理
    memset(&serveraddr, 0, sizeof(serveraddr));  // 对于sockaddr_in结构体，先用 memset() 将结构体的全部字节填充为 0，再给前3个成员赋值，剩下的 sin_zero 自然就是 0 了。
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("192.168.43.58");  // 直接使用本地地址
    serveraddr.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("Bind failure!");
        exit(-1);
    }
    // 让套接字进入被动监听状态，随时监听客户端的连接请求（listen()不会阻塞程序）
    // 所谓被动监听，是指当没有客户端请求时，套接字处于“睡眠”状态，只有当接收到客户端请求时，套接字才会被“唤醒”来响应请求。
    if (listen(listenfd, 5) == -1) {  // 设置请求队列的长度为5
        perror("Listen failure!");
        exit(-1);
    }
    // 初始化peeraddr，用于后续存放客户端套接字（IP:Port）
    memset(&peeraddr, 0, sizeof(peeraddr));
    len = sizeof(peeraddr);
    printf("Server is ready, Waitting for connection.\n");
    // 让处于被动监听状态的套接字接收客户端请求，accept()会阻塞程序执行，直到有请求到来
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &len)) == -1) {
            perror("Accept failure!");
            exit(-1);
        }
        printf("Connection From %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        pthread_create(&tid[0], NULL, send_tcp_segment, param);
        pthread_create(&tid[1], NULL, recv_tcp_segment, param);
        pthread_join(tid[0], &retval[0]);
        pthread_join(tid[1], &retval[1]);
        // while (1) {
        //     // 服务器接收客户端发送的数据
        //     memset(buf, 0, sizeof(buf));
        //     printf("Receiving...\n");
        //     recv(connfd, buf, strlen("close"), 0);
        //     if (strcmp(buf, "close") == 0) {
        //         break;
        //     }

        //     // 服务器往客户端发送数据
        //     memset(buf, 0, sizeof(buf));
        //     printf("Sending...\n");
        //     n = sizeof(struct TcpParam);
        //     memcpy(buf, param, n);
        //     PRINT_TCP_PARAM(param);
        //     //  向客户端套接字中发送数据
        //     send(connfd, buf, n, 0);

        //     sleep(1);
        // }
        printf("Close connection\n");
        close(connfd);
    }
    printf("Shut down server\n");
    close(listenfd);
}