CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O2
INC      = -I../include -DHOST_TEST
SRC      = ../src/sense_scale.c \
           ../src/protect.c \
           ../src/svpwm.c \
           ../src/pwm_front.c \
           ../src/pwm_inv.c \
           ../src/control_dcdc.c \
           ../src/control_inv.c \
           ../src/adc_acq.c \
           test_inverter.c

.PHONY: test clean

test:
	$(CC) $(CFLAGS) $(INC) $(SRC) -lm -o test_inverter
	./test_inverter

clean:
	rm -f test_inverter
