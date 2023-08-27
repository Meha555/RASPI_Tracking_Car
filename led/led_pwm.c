#include<wiringPi.h>
#include<softPwm.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
// #define HARDWARE

int main(void){
    if(wiringPiSetup() < 0){
        perror("Start GPIO Failed.");
	    exit(1);
    }
    #ifdef HARDWARE
    char LED = 1;
    pinMode(LED, PWM_OUTPUT);
    while(1){
        for(int i=0;i<1024;i++){
            pwmWrite(LED,i);
            usleep(1000);
        }
        for(int i=1024;i>0;i--){
            pwmWrite(LED,i);
            usleep(1000);
        }
    }
    #endif
    #ifndef HARDWARE
    char LED = 23;
    pinMode(LED,OUTPUT);
    softPwmCreate(LED,0,100);//创建软件PWM，范围0~100
    while(1){
        for(int i=0;i<100;i++){
            softPwmWrite(LED,i);
            usleep(10000);
        }
        for(int i=100;i>0;i--){
            softPwmWrite(LED,i);
            usleep(10000);
        }
    }
    #endif
}