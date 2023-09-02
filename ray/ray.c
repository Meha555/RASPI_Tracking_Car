#include "ray.h"
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

/*
碰到黑带是高电平，碰到地板是低电平
真值表：
| L   | M1  | M2  | R   |     |
| --- | --- | --- | --- | --- |
| 0   | 0   | 0   | 0   | S   |
| 0   | 0   | 0   | 1   | R   |
| 0   | 0   | 1   | 0   | R   |
| 0   | 0   | 1   | 1   | R   |
| 0   | 1   | 0   | 0   | L   |
| 0   | 1   | 0   | 1   | -   |
| 0   | 1   | 1   | 0   | A   |
| 0   | 1   | 1   | 1   | R   |
| 1   | 0   | 0   | 0   | L   |
| 1   | 0   | 0   | 1   | -   |
| 1   | 0   | 1   | 0   | -   |
| 1   | 0   | 1   | 1   | -   |
| 1   | 1   | 0   | 0   | L   |
| 1   | 1   | 0   | 1   | -   |
| 1   | 1   | 1   | 0   | L   |
| 1   | 1   | 1   | 1   | -   |
*/

struct TCRT5000 tracker = {
    .line_l = 25,
    .line_r = 22,
    .line_m1 = 24,
    .line_m2 = 23,
};

void initial_tcrt5000() {
    pinMode(tracker.line_l, INPUT);
    pinMode(tracker.line_r, INPUT);
    pinMode(tracker.line_m1, INPUT);
    pinMode(tracker.line_m2, INPUT);
}

void tracking() {
    while (1) {
        // 中间都低电平，说明没黑带，可以转弯
        if (digitalRead(tracker.line_m1) == LOW && digitalRead(tracker.line_m2) == LOW) {
            // 左侧高电平，右侧低电平，说明左侧碰到黑带，应该左转
            if (digitalRead(tracker.line_l) == HIGH && digitalRead(tracker.line_r) == LOW) {
                printf("Turn Left permitted.\n");

            }
            // 右侧高电平，左侧低电平，说明右侧碰到黑带，应该右转
            else if (digitalRead(tracker.line_r) == HIGH && digitalRead(tracker.line_l) == LOW) {
                printf("Turn Right permitted.\n");
            }
        }
        // 中间至少有一个为高电平，说明有黑带，可以直行
        else if (digitalRead(tracker.line_m1) == HIGH || digitalRead(tracker.line_m2) == HIGH) {
            // 左右两侧都是低电平，说明可以直走
            if (digitalRead(tracker.line_l) == LOW && digitalRead(tracker.line_r) == LOW) {
                printf("Forward permitted.\n");
            }
        }
        // // 如果中间是高电平，说明碰到黑带
        // if (digitalRead(tracker.line_m1) == HIGH && digitalRead(tracker.line_m2) == HIGH) {
        //     // 左右两侧都是低电平，说明可以直走
        //     if (digitalRead(tracker.line_l) == LOW && digitalRead(tracker.line_r)) {
        //         printf("Forward permitted.\n");
        //     } else {
        //         printf("Invalid signal.\n");
        //     }
        // }
        // // 如果中间低电平，说明需要转弯
        // else {
        //     // 左侧高电平，右侧低电平，说明左侧碰到黑带，应该左转
        //     if (digitalRead(tracker.line_l) == HIGH && digitalRead(tracker.line_r) == LOW) {
        //         printf("Turn Left permitted.\n");
        //     }
        //     // 右侧高电平，左侧低电平，说明右侧碰到黑带，应该右转
        //     else if (digitalRead(tracker.line_r) == HIGH && digitalRead(tracker.line_l) == LOW) {
        //         printf("Turn Right permitted.\n");
        //     }
        // }
        // delay(3000);  // 行进3s
    }
}

// int main() {
//     if (wiringPiSetup() < 0) {
//         perror("Start GPIO Failed.");
//         exit(1);
//     }
//     initial_tcrt5000();
//     tracking();
//     return 0;
// }