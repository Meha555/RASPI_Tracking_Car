#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// #define BCM

int main(void){
    #ifndef BCM
    // printf("In wiringPi code.");
    char LED[3] = {23,24,25};
	if(wiringPiSetup() < 0){
	    perror("Start GPIO Failed.");
	    exit(1);
	}
	for(int i = 0;i<3;i++)
        pinMode(LED[i],OUTPUT);
	while(1){
	    for(int i = 0;i<3;i++){
            digitalWrite(LED[i],1);
            sleep(1);
            digitalWrite(LED[i],0);
        }
	}
    #endif
    #ifdef BCM
    // printf("In BCM code.");
    char LED[3] = {13,19,26};
    if(wiringPiSetupGpio() < 0){
        perror("Start GPIO Failed.");
	    exit(1);
    }
    for(int i = 0;i<3;i++)
        pinMode(LED[i],OUTPUT);
	while(1){
	    for(int i = 0;i<3;i++){
            digitalWrite(LED[i],1);
            sleep(1);
            digitalWrite(LED[i],0);
        }
	}
    #endif
}