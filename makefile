CC = gcc  
CFLAGS = -w -I/usr/include/wiringPi -I/usr/include/pthread  

%.o: %.c  
	$(CC) $(CFLAGS) -c $< -o $@  

main: main.o bcd.o motor.o snoar.o #buzzer.o led.o
	$(CC) $^ -lwiringPi -lpthread -o main  

main.o: main.c  
	$(CC) $(CFLAGS) -c $< -o $@  

motor.o: motor/motor.c  
	$(CC) $(CFLAGS) -c $< -o $@  

bcd.o: bcd/bcd.c  
	$(CC) $(CFLAGS) -c $< -o $@  

snoar.o: sonar/hc_sr04.c
	$(CC) $(CFLAGS) -c $< -o $@  

# buzzer.o: key/buzzer.c  
# 	$(CC) $(CFLAGS) -c $< -o $@

# led.o: led/led.c  
# 	$(CC) $(CFLAGS) -c $< -o $@  

.PHONY: clean
clean:
	$(RM) main *.o