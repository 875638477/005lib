CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O2
INC      = -I../include -DHOST_TEST
SRC      = ../src/sense_scale.c \
           ../src/control_loop.c \
           ../src/protect.c \
           ../src/hrpwm_4ph.c \
           ../src/adc_acq.c \
           test_converter.c

.PHONY: test clean

test:
	$(CC) $(CFLAGS) $(INC) $(SRC) -lm -o test_converter
	./test_converter

clean:
	rm -f test_converter
