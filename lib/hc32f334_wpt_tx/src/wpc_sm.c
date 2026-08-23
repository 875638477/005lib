#include "wpc_sm.h"
#include "app_config.h"

void wpc_sm_init(wpc_sm_t *s)
{
    s->state = WPC_SEL;
    s->no_packet_ms = 0U;
    s->last_cep = 0;
    s->last_prx_mw = 0;
    s->ss_pkt = 0U;
    s->id_pkt = 0U;
    s->cfg_pkt = 0U;
}

void wpc_sm_on_packet(wpc_sm_t *s, uint8_t header, const uint8_t *msg, unsigned n)
{
    s->no_packet_ms = 0U;

    if (s->state == WPC_FAULT) {
        return;
    }

    switch (header) {
    case 0x01: /* Signal Strength */
        s->ss_pkt = 1U;
        if (s->state == WPC_SEL || s->state == WPC_PING) {
            s->state = WPC_ID;
        }
        break;
    case 0x71:
    case 0x81:
        s->id_pkt = 1U;
        if (s->state == WPC_ID || s->state == WPC_PING) {
            s->state = WPC_ID;
        }
        break;
    case 0x51: /* Configuration */
        s->cfg_pkt = 1U;
        if (s->state == WPC_ID) {
            s->state = WPC_NEGO;
        }
        break;
    case 0x03: /* Control Error */
        if (n >= 1U) {
            s->last_cep = (int8_t)msg[0];
        }
        if (s->state == WPC_NEGO || s->state == WPC_ID || s->state == WPC_PT) {
            s->state = WPC_PT;
        }
        break;
    case 0x04: /* Received Power 8-bit */
        if (n >= 1U) {
            s->last_prx_mw = (int32_t)msg[0] * 500;
        }
        if (s->state == WPC_NEGO || s->state == WPC_PT) {
            s->state = WPC_PT;
        }
        break;
    case 0x31: /* Received Power 16-bit 近似 */
        if (n >= 2U) {
            s->last_prx_mw = ((int32_t)msg[0] << 8) | msg[1];
        }
        if (s->state == WPC_NEGO || s->state == WPC_PT) {
            s->state = WPC_PT;
        }
        break;
    case 0x02: /* End Power Transfer */
        s->state = WPC_SEL;
        s->ss_pkt = 0U;
        s->id_pkt = 0U;
        s->cfg_pkt = 0U;
        break;
    default:
        break;
    }
}

void wpc_sm_tick_ms(wpc_sm_t *s, uint32_t dt_ms)
{
    if (s->state == WPC_SEL || s->state == WPC_FAULT) {
        return;
    }
    s->no_packet_ms += dt_ms;
    if (s->state == WPC_PT && s->no_packet_ms > COMM_TIMEOUT_MS) {
        s->state = WPC_SEL;
    }
}

const char *wpc_state_name(wpc_state_t st)
{
    switch (st) {
    case WPC_SEL:   return "selection";
    case WPC_PING:  return "ping";
    case WPC_ID:    return "id";
    case WPC_NEGO:  return "nego";
    case WPC_PT:    return "power_transfer";
    case WPC_FAULT: return "fault";
    default:        return "unknown";
    }
}
