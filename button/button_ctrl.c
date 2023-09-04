#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "button.h"

void initial_button() {
    pinMode(BUTTON_PIN, INPUT);
}

void button_action(void(*fp)(int)) {
    // 按下是高电平，弹起是低电平
    while (1) {
        if (digitalRead(BUTTON_PIN) == HIGH) {      // 初次检测
            delay(20);                              // 延迟20ms防抖
            if (digitalRead(BUTTON_PIN) == HIGH) {  // 再次检测
                printf("Long press for 3 seconds to shut down!\n");
                delay(3000);  // 等待3s
                if (digitalRead(BUTTON_PIN) == HIGH) {
                    pinMode(BUTTON_PIN, LOW);
                    printf("Shuting down~\n");
                    (*fp)(2);
                    system("sudo halt");
                } else
                    printf("Keypress time is too short!\n");
            }
        }
    }
}

// int main(void) {
//     if (wiringPiSetup() < 0) {
//         perror("Start GPIO Failed.");
//         exit(1);
//     }
//     pinMode(BUTTON_PIN, INPUT);
//     // 按下是高电平，弹起是低电平
//     while (1) {
//         if (digitalRead(BUTTON_PIN) == HIGH) {      // 初次检测
//             delay(20);                              // 延迟20ms防抖
//             if (digitalRead(BUTTON_PIN) == HIGH) {  // 再次检测
//                 delay(3000);                        // 等待3s
//                 if (digitalRead(BUTTON_PIN) == HIGH) {
//                     pinMode(BUTTON_PIN, LOW);
//                     system("shutdown -h now");
//                 }
//             }
//         }
//     }
//     return 0;
// }