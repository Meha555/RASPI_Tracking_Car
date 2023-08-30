#ifndef MOTOR_H
#define MOTOR_H

extern void inital_drive();
extern void drive(double (*fp1)(void),void (*fp2)(int,int,int,int,int));
// void bothside_ahead_speedup();
// void bothside_ahead_slowdown();
// void bothside_goback_speedup();
// void bothside_goback_slowdown();
// void onside_ahead_speedup(enum Direct direct);
// void onside_goback_speedup(enum Direct direct);
// void onside_ahead_slowdown(enum Direct direct);
// void onside_goback_slowdown(enum Direct direct);
// void drive_break();

#endif  // MOTOR_H