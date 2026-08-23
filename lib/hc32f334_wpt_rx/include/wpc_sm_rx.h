#ifndef WPC_SM_RX_H
#define WPC_SM_RX_H

#include <stdint.h>

typedef enum {
    RX_IDLE = 0,
    RX_SS,
    RX_ID,
    RX_PT,
    RX_FAULT
} rx_state_t;

typedef struct {
    rx_state_t state;
    int16_t cep;
    int32_t prx_mw;
    uint8_t ss_sent;
    uint8_t id_sent;
    uint8_t ept;
} wpc_sm_rx_t;

void wpc_sm_rx_init(wpc_sm_rx_t *s);
void wpc_sm_rx_on_field(wpc_sm_rx_t *s, uint8_t present);
void wpc_sm_rx_update_cep(wpc_sm_rx_t *s, int16_t cep, int32_t prx_mw);
void wpc_sm_rx_request_ept(wpc_sm_rx_t *s);
const char *wpc_sm_rx_name(rx_state_t st);

#endif
