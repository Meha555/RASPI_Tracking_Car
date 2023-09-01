#include "bcd.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

/*
> [TM1637驱动数码管_笑面浮屠的博客-CSDN博客](https://blog.csdn.net/q1241580040/article/details/45815245)

微处理器的数据通过两线总线接口和TM1637通信，采用I2C总线协议：

- 在输入数据时当CLK是高电平时, DIO上的信号必须保持不变;
- 只有CLK上的时钟信号为低电平时，DIO上的信号才能改变。【CKL周期中间那一段底电平是用来DIO传输数据的】
- 开始数据传输的条件：CLK为高电平时，DIO由高电平变低电平;
- 结束数据传输的条件：CLK为高电平时，DIO由低电平变为高电平。
- 传输数据：在CLK下降沿后由DIO输入的第一个字节作为一条指令。
- 当正确传输8位数据后TM1637会第九个时钟在DIO管脚上给出一个ACK信号，把DIO拉低。
- 手动捕获硬件自动发出的ACK信号，实现同步
*/

#define BCD_ADDR_1 0xc0
#define BCD_ADDR_2 0xc1
#define BCD_ADDR_3 0xc2
#define BCD_ADDR_4 0xc3

#define CMD_WRITE 0x40
#define CMD_FIXADDR 0x44
#define CMD_VISIBLE 0x88

static char seg_data[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};  // LOW~9 + 0x80 就能显示冒号

static void cleanup_pin(int sig) {
    pinMode(DIO_PIN, INPUT);
    pinMode(CLK_PIN, INPUT);
    // signal(SIGINT, SIG_DFL);
}
void tm1637_init() {
    pinMode(CLK_PIN, OUTPUT);
    pinMode(DIO_PIN, OUTPUT);
    // signal(SIGINT, cleanup_pin);
}

// 开始一次数据传输
void tm1637_start() {
    // 拉高CLK，准备开始制造DIO下降沿
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(140);
    // 制造DIO下降沿
    digitalWrite(DIO_PIN, HIGH);
    delayMicroseconds(140);
    digitalWrite(DIO_PIN, LOW);
    delayMicroseconds(140);
    // 拉低CLK，之后可以开始数据传输
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(140);
}
// 结束一次数据传输
void tm1637_stop() {
    // 结束前的准备：拉低CLK，防止CLK为高电平时DIO下降沿被误解为开始信号
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(140);
    digitalWrite(DIO_PIN, LOW);  // 保险起见，将DIO拉低
    delayMicroseconds(140);
    // 拉高CLK和DIO，表示数据传输结束
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(140);
    digitalWrite(DIO_PIN, HIGH);
    delayMicroseconds(140);
}
// 写入1位数据
void write_bit(char bit) {
    digitalWrite(CLK_PIN, LOW);  // 将时钟线（CLK_PIN）设置为低电平，准备开始一次数据传输
    delayMicroseconds(140);      // 等待140微秒，给芯片足够的准备时间。
    digitalWrite(DIO_PIN, bit);
    // if (bit)
    //     digitalWrite(DIO_PIN, HIGH);
    // else
    //     digitalWrite(DIO_PIN, LOW);
    delayMicroseconds(140);       // 等待140微秒，给芯片足够的传输时间。
    digitalWrite(CLK_PIN, HIGH);  // 将CLK设置为高电平，结束当前bit的传输
}
// 写入1个字节的数据
void write_byte(char data) {
    char i = LOW;
    for (i = LOW; i < 8; i++)
        write_bit((data >> i) & 1);  // 提取出最后一位bit
    // 开始处理ACK
    // 拉低CLK，表示开始传输数据，进入数据传输状态
    digitalWrite(CLK_PIN, LOW);
    // 延迟140微秒，确保芯片接收完毕信号
    delayMicroseconds(140);
    // 拉高DIO，制造电位差，提高后面检测DIO引脚的灵敏度
    digitalWrite(DIO_PIN, HIGH);
    // 延迟140微秒，确保芯片接收完毕信号
    delayMicroseconds(140);
    // 拉高CLK，表示数据传输结束，结束数据传输状态
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(140);
    pinMode(DIO_PIN, INPUT);                          // 临时设置数据线为输入模式，从而检测数据线的电平
    int timeout = 1000;                               // 添加超时时间，防止信号波动导致信号丢失
    while (digitalRead(DIO_PIN) && timeout != LOW) {  // 等待数据线变为低电平，表示芯片已经接收完所有的数据，然后将数据线重新设置为输出模式。
        timeout--;
        delayMicroseconds(HIGH);
    }
    pinMode(DIO_PIN, OUTPUT);
}
// 写命令
void write_command(char cmd) {
    tm1637_start();
    write_byte(cmd);
    tm1637_stop();
}
// 写数据
void write_data(char addr, char data) {
    // 采用固定地址
    tm1637_start();
    write_byte(addr);  // 先写地址命令
    write_byte(data);  // 再写数据
    tm1637_stop();
}
void bcd_display(int h_shi, int h_ge, int m_shi, int m_ge, int flag) {
    write_command(CMD_WRITE);                 // 写数据
    write_command(CMD_FIXADDR);               // 固定地址
    write_data(BCD_ADDR_1, seg_data[h_shi]);  // 写第一个数码管--小时的十位
    write_data(BCD_ADDR_2, seg_data[h_ge] + flag * 128);
    write_data(BCD_ADDR_3, seg_data[m_shi]);
    write_data(BCD_ADDR_4, seg_data[m_ge]);
    write_command(CMD_VISIBLE);  // 显示开
}

struct tm* get_time() {
    time_t timep;
    struct tm* p;
    time(&timep);
    return gmtime(&timep);
}