#ifndef HC_SR04_H
#define HC_SR04_H

#define ECHO_PIN 4
#define TRIG_PIN 5

extern void inital_hr024();
extern double get_distance();

#endif  // HC_SR04_H