SRC = /home/pi/Desktop/raspi
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
