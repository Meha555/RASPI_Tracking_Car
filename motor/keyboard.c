#include <stdio.h>
#include <termio.h>
/*
termios是UNIX系统中用来控制终端I/O的接口
c_lflag字段是termios结构体中的一个成员，它控制了终端的本地模式标志。
*/
int main() {
    struct termios tms_old, tms_new;  // 声明终端属性结构体
    tcgetattr(0, &tms_old);           // 获取旧的终端属性
    // 设置新的终端属性
    tms_new = tms_old;                    // 复制终端属性
    tms_new.c_lflag &= ~(ICANON | ECHO);  // 禁用了标准输入的行缓冲（通过取反ICANON）和回显（通过取反ECHO）
    tcsetattr(0, TCSANOW, &tms_new);      // 设置新的终端属性
    while (1) {
        unsigned char ch = getchar();  // 读取用户输入的字符
        printf("按下的是: %c\n", ch);  // 打印字符的 ASCII 值
    }
    tcsetattr(0, TCSANOW, &tms_old);  // 恢复旧的终端属性
    return 0;
}
