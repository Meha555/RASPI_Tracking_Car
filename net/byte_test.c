#include <stdio.h>
#include <string.h>

/**
 * @description: 判断当前主机字节序
 * @return 返回0小端字节序，返回1大端字节序
 */
int dGetHostByteOrder() {
    unsigned int a = 0x12345678;
    unsigned char* p = (unsigned char*)(&a);
    if (*p == 0x78) {
        return 0;
    } else {
        return 1;
    }
}
// 24 bytes
struct TcpParam {
    unsigned char buzzer_pin;        // 1 byte
    unsigned char keyctrl_switcher;  // 1 byte
    struct {
        unsigned char key_pressed;  // 1 byte
        int dist;                   // 4 bytes
        enum Orientation { AHEAD,
                           TURN_LEFT,
                           TURN_RIGHT,
        } orient;   // 4 bytes
    } motor_param;  // 12 bytes
    struct {
        float temperature;
        float humidity;
    } dht11_param;  // 8 bytes
};
int main(void) {
    printf("当前主机字节序: %d\n", dGetHostByteOrder());
    printf("char %d\n", sizeof(char));
    printf("short %d\n", sizeof(short));
    printf("int %d\n", sizeof(int));
    printf("long %d\n", sizeof(long));
    printf("long long %d\n", sizeof(long long));
    printf("float %d\n", sizeof(float));
    printf("double %d\n", sizeof(double));
    printf("Size %d\n", sizeof(struct TcpParam));
    unsigned char c = 1;
    printf("%d\n", c);
    c ^= 1;
    printf("%d\n", c);
    return 0;
}
