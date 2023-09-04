#ifndef BUZZER_H
#define BUZZER_H

#define BEEP_PIN 0  // 蜂鸣器IO口
#define BUTTON_PIN 21  // 按键OUT口

#define DOUBLE_BEEP     \
    do {                \
        buzzer_beep(1); \
        buzzer_beep(0); \
        delay(10);      \
        buzzer_beep(1); \
        buzzer_beep(0); \
    } while (0);

extern void initial_buzzer();
extern void buzzer_beep(int flag);
extern void command_shut(int sig);

#endif  // BUZZER_H