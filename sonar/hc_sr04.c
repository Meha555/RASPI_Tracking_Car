#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>
#include "hc_sr04.h"

void inital_hr024() {
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    pinMode(ECHO_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);
    digitalWrite(TRIG_PIN, LOW);
}

double get_distance() {
    // 拉低TRIG，准备制作开始信号脉冲
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(140);
    // 制作开始信号脉冲
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);  // 用持续10微秒的TTL脉冲表示开始
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(140);  // 等一下更精确
    while (digitalRead(ECHO_PIN) == LOW)
        ;  // 等待ECHO引脚返回高电平信号
    // 接收到高电平信号，立即开始计时
    time_t t1 = micros();
    while (digitalRead(ECHO_PIN) == HIGH)
        ;  // 等待ECHO引脚的高电平信号结束
    time_t t2 = micros();
    return (t2 - t1) * 340 * 100 / 2e6;
}

// int main() {
//     inital_hr024();
//     while (1) {
//         double dist = get_distance();
//         if (dist >= 400)
//             printf("Lose of precision!\n");
//         else
//             printf("Dectecting dist: %lf CM\n", dist);
//         delay(500);
//     }
//     return 0;
// }