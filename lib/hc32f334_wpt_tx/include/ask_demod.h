#ifndef ASK_DEMOD_H
#define ASK_DEMOD_H

#include <stdint.h>

#define ASK_BIT_US              500U
#define ASK_PREAMBLE_ONES       11U
#define ASK_MAX_BYTES           32U

typedef enum {
    ASK_IDLE = 0,
    ASK_PREAMBLE,
    ASK_DATA
} ask_state_t;

typedef struct {
    ask_state_t state;
    uint8_t last_level;
    uint8_t mid_trans;
    uint8_t bit_count;
    uint8_t byte_acc;
    uint8_t bytes[ASK_MAX_BYTES];
    uint8_t nbytes;
    uint8_t packet_ready;
} ask_demod_t;

void ask_demod_init(ask_demod_t *d);
void ask_demod_reset(ask_demod_t *d);
/* level: 1 = 高包络, 0 = 低包络。每半比特（250 µs）调一次。 */
int ask_demod_halfbit(ask_demod_t *d, uint8_t level);
int ask_packet_ok(const uint8_t *bytes, unsigned n);
uint8_t ask_checksum(const uint8_t *bytes, unsigned n);

#endif
