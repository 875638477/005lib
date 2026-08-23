#ifndef WPC_SM_H
#define WPC_SM_H

#include <stdint.h>

typedef enum {
    WPC_SEL = 0,
    WPC_PING,
    WPC_ID,
    WPC_NEGO,
    WPC_PT,
    WPC_FAULT
} wpc_state_t;

typedef struct {
    wpc_state_t state;
    uint32_t no_packet_ms;
    int16_t last_cep;
    int32_t last_prx_mw;
    uint8_t ss_pkt;         /* 收到 0x01 */
    uint8_t id_pkt;
    uint8_t cfg_pkt;
} wpc_sm_t;

void wpc_sm_init(wpc_sm_t *s);
void wpc_sm_on_packet(wpc_sm_t *s, uint8_t header, const uint8_t *msg, unsigned n);
void wpc_sm_tick_ms(wpc_sm_t *s, uint32_t dt_ms);
const char *wpc_state_name(wpc_state_t st);

#endif
