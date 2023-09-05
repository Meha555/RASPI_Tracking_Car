#include "raspi_tcp.h"

int listenfd, connfd;
struct sockaddr_in serveraddr, peeraddr;
socklen_t len;
char buf[BUFF_SIZE];
ssize_t n;

// void send_tcp_segment() {
// }

// void recv_tcp_segment() {
// }

void setup_tcpserver(struct TcpParam* param) {
    // 创建监听客户端的套接字 - 确定套接字的各种属性（地址类型、协议族）
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {  // 使用IPv4地址 面向字节流和连接的协议 指定是TCP协议（也可以自动推导）
        perror("Socket Create Failure!");
        exit(-1);
    }
    // 将套接字与特定的IP地址和端口绑定起来，使得来自该IP地址并经过该端口的数据交由该套接字处理
    memset(&serveraddr, 0, sizeof(serveraddr));  // 对于sockaddr_in结构体，先用 memset() 将结构体的全部字节填充为 0，再给前3个成员赋值，剩下的 sin_zero 自然就是 0 了。
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(INADDR_ANY);  // 直接使用本地地址
    serveraddr.sin_port = htons(atoi(PORT));
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
    // 让处于被动监听状态的套接字接收客户端请求，accept()会阻塞程序执行，直到有请求到来
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &len)) == -1) {
            perror("Accept failure!");
            exit(-1);
        }
        printf("From %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        while (1) {
            // 创建接收数据的缓冲区
            memset(buf, 0, sizeof(buf));
            // TODO - 获取TcpParam中的数据（包括按键、各设备开关引脚、湿温度数据、）
            n = sizeof(struct TcpParam);
            memcpy(buf, param, n);
            //  向客户端套接字中发送数据
            send(connfd, buf, n, 0);
        }
        close(connfd);
    }
}