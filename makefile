CC = gcc  
CFLAGS = -w -I/usr/include/wiringPi -I/usr/include/pthread -g  
  
%.o: %.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
main: main.o bcd.o motor.o snoar.o actuator.o thread_tools.o #buzzer.o led.o  
	$(CC) $^ -lwiringPi -lpthread -o main  
  
main.o: main.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
motor.o: motor/motor.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
bcd.o: bcd/bcd.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
snoar.o: sonar/hc_sr04.c  
	$(CC) $(CFLAGS) -c $< -o $@  

actuator.o: actuator/actuator.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
# buzzer.o: key/buzzer.c  
# 	$(CC) $(CFLAGS) -c $< -o $@  
  
# led.o: led/led.c  
# 	$(CC) $(CFLAGS) -c $< -o $@  

thread_tools.o: utils/thread_tools.c  
	$(CC) $(CFLAGS) -c $< -o $@  
  
.PHONY: clean  
clean:  
	$(RM) main *.o