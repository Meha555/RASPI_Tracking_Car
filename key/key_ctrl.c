#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define KEY_PIN 21

int main(void) {
    if (wiringPiSetup() < 0) {
        perror("Start GPIO Failed.");
        exit(1);
    }
    pinMode(KEY_PIN, INPUT);
    // 按下是高电平，弹起是低电平
    while (1) {
        if (digitalRead(KEY_PIN) == HIGH) {      // 初次检测
            delay(20);                           // 延迟20ms防抖
            if (digitalRead(KEY_PIN) == HIGH) {  // 再次检测
                delay(3000);                     // 等待3s
                if (digitalRead(KEY_PIN) == HIGH) {
                    pinMode(KEY_PIN, LOW);
                    system("shutdown -h now");
                }
            }
        }
    }
    return 0;
}