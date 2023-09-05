#include <stdio.h>
#include <string.h>

int main() {
    char str1[3] = "abc";
    char str2[3] = "abc";
    printf("%d\n", memcmp(str1, str2, 3));
    // if () {
    //     puts("Y");
    // } else
    //     puts("N");
    return 0;
}