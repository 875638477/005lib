CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O2
INC      = -I../include -I../../hc32f334_wpt_tx/include -DHOST_TEST
SRC      = ../src/rx_math.c \
           ../src/hrpwm_sr.c \
           ../src/hrpwm_buck.c \
           ../src/sense_scale.c \
           ../src/protect.c \
           ../src/wpc_sm_rx.c \
           ../src/ask_mod.c \
           ../../hc32f334_wpt_tx/src/ask_demod.c \
           test_rx.c

.PHONY: test clean

test:
	$(CC) $(CFLAGS) $(INC) -DWPT_POWER_W=20 $(SRC) -lm -o test_rx
	./test_rx

clean:
	rm -f test_rx
