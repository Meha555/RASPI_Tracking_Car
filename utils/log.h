#ifndef LOG_H
#define LOG_H

#define LOG_FILE "/home/pi/Desktop/log.txt"

int fd = -1;

int initLog();
void printLog(char* str, int len);
void closeLog();

#endif  // LOG_H