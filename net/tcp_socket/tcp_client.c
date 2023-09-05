#include "tcp_header.h"

int main(int argc, char* argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, myaddr;
    char buf[BUFF_SIZE];
    if (argc < 3) {
        printf("Usage: %s IP Port\n", argv[0]);
        return 0;
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {  // 使用IPv4地址 面向字节流和连接的协议 指定是TCP协议（也可以自动推导）
        perror("Socket Create Failure!");
        exit(-1);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {  // 连接到服务器
        perror("Socket Connect Failure!");
        exit(-1);
    }
    printf("> ");
    while (fgets(buf, BUFF_SIZE, stdin) != NULL) {  // 从标准输入流获取一行字符串
        // abc\n --- a b c \n \0----
        send(sockfd, buf, strlen(buf), 0);
        memset(buf, 0, sizeof(buf));
        recv(sockfd, buf, BUFF_SIZE, 0);
        printf("%s\n", buf);  // 写回到标准输出流，使之在屏幕上显示
        printf("> ");
    }
    close(sockfd);
    return 0;
}