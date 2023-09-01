#ifndef ACTUATOR_H
#define ACTUATOR_H

#define SERVO_PIN 7

extern void initial_actuator();
extern void control_spin();
extern int get_duty_cycle(int);
extern void command_reset(int);

#endif  // ACTUATOR_H