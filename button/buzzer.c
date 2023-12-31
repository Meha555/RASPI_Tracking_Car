#include "buzzer.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

/*
 检测出键闭合后执行一个延时程序，5ms～10ms（取决于机械特性）的延时，
 让前沿抖动消失后再一次检测键的状态，如果仍保持闭合状态电平，则确认为真正有键按下。
 当检测到按键释放后，也要给5ms～10ms的延时，待后沿抖动消失后才能转入该键的处理程序
*/

void command_shut(int sig) {
    printf("Buzzer shutup.\n");
    digitalWrite(BEEP_PIN, LOW);
    // signal(SIGINT, SIG_DFL);
}

void initial_buzzer() {
    // if (wiringPiSetup() < 0) {
    //     perror("Start GPIO Failed.");
    //     exit(1);
    // }
    // signal(SIGINT, command_shut);
    pinMode(BEEP_PIN, OUTPUT);
    digitalWrite(BEEP_PIN, LOW);
    pinMode(BUTTON_PIN, INPUT);
}

void buzzer_single_beep(int flag) {
    if (flag)
        digitalWrite(BEEP_PIN, HIGH);
    else
        digitalWrite(BEEP_PIN, LOW);
}

void buzzer_control() {
    while (1) {
        if (digitalRead(BUTTON_PIN) == HIGH) {      // 初次检测
            delay(20);                              // 延迟20ms防抖
            if (digitalRead(BUTTON_PIN) == HIGH) {  // 再次检测
                printf("Key pin: %d\n", digitalRead(BUTTON_PIN));
                while ((digitalRead(BUTTON_PIN) == HIGH))
                    ;  // 等待按键弹起
                // 此时就是真正的按键弹起后了
                printf("Key pin: %d\n", digitalRead(BUTTON_PIN));
                if (digitalRead(BEEP_PIN) == LOW)
                    digitalWrite(BEEP_PIN, HIGH);
                else
                    digitalWrite(BEEP_PIN, LOW);
            }
        }
    }
}

// int main(void) {
//     initial_buzzer();
//     while (1) {
//         if (digitalRead(BUTTON_PIN) == HIGH) {      // 初次检测
//             delay(20);                              // 延迟20ms防抖
//             if (digitalRead(BUTTON_PIN) == HIGH) {  // 再次检测
//                 printf("Key pin: %d\n", digitalRead(BUTTON_PIN));
//                 while ((digitalRead(BUTTON_PIN) == HIGH))
//                     ;  // 等待按键弹起
//                 // 此时就是真正的按键弹起后了
//                 printf("Key pin: %d\n", digitalRead(BUTTON_PIN));
//                 if (digitalRead(BEEP_PIN) == LOW)
//                     digitalWrite(BEEP_PIN, HIGH);
//                 else
//                     digitalWrite(BEEP_PIN, LOW);
//             }
//         }
//     }
//     pinMode(BEEP_PIN, LOW);
//     pinMode(BUTTON_PIN, LOW);
//     return 0;
// }