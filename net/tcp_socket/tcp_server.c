#include "tcp_header.h"

int main(int argc, char* argv[]) {
    int listenfd, connfd;                     // listenfd是服务器套接字，用于监听；connfd是存储客户端的套接字，用于与连接上的客户端通信
    struct sockaddr_in serveraddr, peeraddr;  // serveraddr是服务器自身socket地址，peeraddr是对端socket地址
    socklen_t len;
    char buf[BUFF_SIZE] = {0};
    ssize_t n;

    if (argc < 3) {
        printf("Usage: %s IP Port\n", argv[0]);
        return 0;
    }
    // 创建监听客户端的套接字 - 确定套接字的各种属性（地址类型、协议族）
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {  // 使用IPv4地址 面向字节流和连接的协议 指定是TCP协议（也可以自动推导）
        perror("Socket Create Failure!");
        exit(-1);
    }
    // 将套接字与特定的IP地址和端口绑定起来，使得来自该IP地址并经过该端口的数据交由该套接字处理
    memset(&serveraddr, 0, sizeof(serveraddr));  // 对于sockaddr_in结构体，先用 memset() 将结构体的全部字节填充为 0，再给前3个成员赋值，剩下的 sin_zero 自然就是 0 了。
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));  // INADDR_ANY
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
            // 从客户端套接字中读取数据
            n = recv(connfd, buf, BUFF_SIZE, 0);  // a b c \n n=4
            if (n == 0)                           // 接收到一个长度为0的消息时，这意味着客户端已经关闭了连接，所以退出消息接收的循环
                break;
            buf[n] = '\0';
            printf("n = %d %s", n, buf);  // 写回到标准输出流，使之在屏幕上显示
            // 向客户端套接字中发送数据
            send(connfd, buf, n, 0);
        }
        close(connfd);
    }
    return 0;
}
