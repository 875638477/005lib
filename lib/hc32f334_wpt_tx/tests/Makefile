CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -O2
INC      = -I../include -DHOST_TEST
SRC      = ../src/wpt_math.c \
           ../src/hrpwm_inv.c \
           ../src/sense_scale.c \
           ../src/protect.c \
           ../src/power_ctrl.c \
           ../src/wpc_sm.c \
           ../src/ask_demod.c \
           test_wpt.c

.PHONY: test test30 test50 clean

test:
	$(CC) $(CFLAGS) $(INC) -DWPT_POWER_W=20 $(SRC) -lm -o test_wpt
	./test_wpt

test30:
	$(CC) $(CFLAGS) $(INC) -DWPT_POWER_W=30 $(SRC) -lm -o test_wpt30
	@echo "built 30 W profile"

test50:
	$(CC) $(CFLAGS) $(INC) -DWPT_POWER_W=50 $(SRC) -lm -o test_wpt50
	@echo "built 50 W profile"

clean:
	rm -f test_wpt test_wpt30 test_wpt50
