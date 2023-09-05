SRC = /home/pi/Desktop/raspi
LIB = 
CFLAG = #-L$(SRC) -l$(LIB)

SOURCES = $(wildcard *.c)

EXECUTABLES = $(SOURCES:.c=)

all: $(EXECUTABLES)

%: %.c
	gcc $< -o $@ $(CFLAG)

.PHONY: clean
clean:
	$(RM) $(EXECUTABLES)
