TARGET = keyboard motor_bcm
SRC = /home/pi/Desktop/respberry_-tracking_-car
LIB = wiringPi
CFLAG = -L$(SRC) -l$(LIB)

SOURCES = $(wildcard *.c)

EXECUTABLES = $(SOURCES:.c=)

all: $(EXECUTABLES)

%: %.c
	gcc $< -o $@ $(CFLAG)

.PHONY: clean
clean:
	$(RM) $(EXECUTABLES)
