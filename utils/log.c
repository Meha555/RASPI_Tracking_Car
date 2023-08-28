#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"

int initLog(){
    // 设置文件权限为所有者可读写，组和其他用户只读
    mode_t fileMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = open("/home/pi/Desktop/a.txt",O_RDWR | O_TRUNC | O_CREAT, fileMode);
}
void printLog(char* str,int len){
    write(fd,str,len);
}
void closeLog(){
    close(fd);
}