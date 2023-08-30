#include <pthread.h>
#include <stdio.h>
#include "bcd/bcd.h"
#include "key/buzzer.h"
#include "led/led.h"
#include "motor/motor.h"
#include "sonar/hc_sr04.h"

#define THREAD_NUM 1
pthread_t tid[THREAD_NUM];

// void* bcd_thread(void* args) {
//     // printf("In bcd_thread.\n");
//     inital_hr024();
//     while (1) {
//         int dist = get_distance();
//         bcd_display(0, dist / 100, dist % 100 / 10, dist % 10, 0);
//         delay(500);
//     }
// }

void* motor_thread(void* args) {
    printf("In motor_thread.\n");
    inital_hr024();
    drive(get_distance, bcd_display);
}

// void* sonar_thread(void* args) {
//     while (1) {
//         double dist = get_distance();
//         if (dist >= 400)
//             printf("Lose precision!\n");
//         else
//             printf("Dectecting dist: %lf CM\n", dist);
//         delay(500);
//     }
// }

int main(int argc, char argv[]) {
    // pthread_create(&tid[0], NULL, bcd_thread, NULL);
    pthread_create(&tid[0], NULL, motor_thread, NULL);
    // pthread_create(&tid[2], NULL, sonar_thread, NULL);
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(tid[i], NULL);
    }
    return 0;
}