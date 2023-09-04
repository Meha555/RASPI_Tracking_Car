#include "dht11.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

void initial_dht11() {
    pinMode(DATA_PIN, OUTPUT);  // 设置引脚为输出，用于主机发送开始信号
    delay(30);                  // 延迟30us等待dht11响应
    digitalWrite(DATA_PIN, HIGH);
    delay(30);  // 延迟30us等待dht11响应
}

void dht11_reset() {
    initial_dht11();
    // 发出开始信号，由高电平变为低电平保持>=18ms后再变回高电平
    digitalWrite(DATA_PIN, LOW);
    delay(18);
    digitalWrite(DATA_PIN, HIGH);
    delayMicroseconds(40);     // 延迟20~40us等待dht11响应
    pinMode(DATA_PIN, INPUT);  // 设置引脚为输入，用于主机接收信号
}

unsigned char read_bit() {
    time_t t1, t2;
    while (digitalRead(DATA_PIN) == LOW)
        ;                                 // 等待一位数据的前置低电平结束（50us）
    delayMicroseconds(28);                // 取26us~28us的最长时间等待
    if (digitalRead(DATA_PIN) == HIGH) {  // 26us~28us后判断是否是高电平，如果是高电平则为1，低电平则为0
        t1 = time(NULL);
        while (digitalRead(DATA_PIN) == HIGH) {  // 等待高电平结束
            t2 = time(NULL);
            if (t2 - t1)  // time返回的单位是s
                return 2;
        }
        return 1;
    } else
        return 0;
}

unsigned char read_byte() {
    unsigned char data = 0, retval = 0;
    for (int i = 0; i < 8; i++) {
        data <<= 1;  // 向高位左移，空出最低位
        retval = read_bit();
        if (retval == 2)  // 超时
            return 0xFF;
        else
            data |= retval;  // 将读取到的位放在刚刚空出的最低位
    }
    return data;
}

// 读取40位数据
int dht11_read_data(unsigned char buff[]) {
    dht11_reset();
    if (digitalRead(DATA_PIN) == LOW) {  // 如果是在输入模式
        while (digitalRead(DATA_PIN) == LOW)
            ;  // 等待DHT响应时间结束（80us）
        while (digitalRead(DATA_PIN) == HIGH)
            ;  // 等待DHT拉高时间结束（80us）
        // 开始读取数据（一共5字节40位）
        for (int i = 0; i < 5; i++) {
            buff[i] = read_byte();
            if (buff[i] == 0xFF)  // 超时
                return 2;
        }
        while (digitalRead(DATA_PIN) == LOW)
            ;
        pinMode(DATA_PIN, OUTPUT);
        digitalWrite(DATA_PIN, HIGH);
        unsigned char checksum = buff[0] + buff[1] + buff[2] + buff[3];
        return checksum == buff[4];
    }
    return 3;
}

// int main() {
//     if (wiringPiSetup() < 0) {
//         perror("Start GPIO Failed.");
//         exit(1);
//     }
//     unsigned char data[5] = {0};
//     while (1) {
//         switch (dht11_read_data(data)) {
//             case 0:
//                 printf("Checksum Error! %d %d %d %d %d\n", data[0], data[1], data[2], data[3], data[4]);
//                 break;
//             case 1:
//                 printf("Humidity = %d.%d, Temperature = %d.%d\n", data[0], data[1], data[2], data[3]);
//                 break;
//             case 2:
//                 printf("Time Out!\n");
//                 break;
//             case 3:
//                 printf("DHT11 no response!\n");
//                 break;
//         }
//         delay(1000);
//     }
//     return 0;
// }