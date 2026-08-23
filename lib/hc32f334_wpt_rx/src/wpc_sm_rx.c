#include "wpc_sm_rx.h"

void wpc_sm_rx_init(wpc_sm_rx_t *s)
{
    s->state = RX_IDLE;
    s->cep = 0;
    s->prx_mw = 0;
    s->ss_sent = 0U;
    s->id_sent = 0U;
    s->ept = 0U;
}

void wpc_sm_rx_on_field(wpc_sm_rx_t *s, uint8_t present)
{
    if (!present) {
        s->state = RX_IDLE;
        s->ss_sent = 0U;
        s->id_sent = 0U;
        s->ept = 0U;
        return;
    }
    if (s->state == RX_FAULT) {
        return;
    }
    if (s->state == RX_IDLE) {
        s->state = RX_SS;
    }
}

void wpc_sm_rx_update_cep(wpc_sm_rx_t *s, int16_t cep, int32_t prx_mw)
{
    s->cep = cep;
    s->prx_mw = prx_mw;
    if (s->state == RX_SS) {
        s->ss_sent = 1U;
        s->state = RX_ID;
    } else if (s->state == RX_ID) {
        s->id_sent = 1U;
        s->state = RX_PT;
    } else if (s->state == RX_PT) {
        /* stay */
    }
}

void wpc_sm_rx_request_ept(wpc_sm_rx_t *s)
{
    s->ept = 1U;
    s->state = RX_IDLE;
}

const char *wpc_sm_rx_name(rx_state_t st)
{
    switch (st) {
    case RX_IDLE:  return "idle";
    case RX_SS:    return "signal_strength";
    case RX_ID:    return "id";
    case RX_PT:    return "power_transfer";
    case RX_FAULT: return "fault";
    default:       return "unknown";
    }
}
