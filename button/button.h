#ifndef BUTTON_H
#define BUTTON_H

#define BUTTON_PIN 21

extern void initial_button();
extern void button_action(void (*fp)(int));

#endif  // BUTTON_H