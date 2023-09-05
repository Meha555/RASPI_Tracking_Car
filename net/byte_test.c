#include <stdio.h>
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

int main(void) {
    printf("当前主机字节序: %d\n", dGetHostByteOrder());
    printf("char %d\n", sizeof(char));
    printf("short %d\n", sizeof(short));
    printf("int %d\n", sizeof(int));
    printf("long %d\n", sizeof(long));
    printf("long long %d\n", sizeof(long long));
    printf("float %d\n", sizeof(float));
    printf("double %d\n", sizeof(double));
    return 0;
}