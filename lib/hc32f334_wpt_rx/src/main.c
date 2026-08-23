#include "app_config.h"
#include "board_pin.h"
#include "hrpwm_sr.h"
#include "hrpwm_buck.h"
#include "sense_scale.h"
#include "protect.h"
#include "wpc_sm_rx.h"
#include "rx_math.h"
#include "ask_mod.h"

#ifndef HOST_TEST
int wpt_rx_app(void)
{
    hrpwm_sr_t sr;
    hrpwm_buck_t buck;
    protect_t prot;
    wpc_sm_rx_t sm;
    sense_raw_t raw;
    sense_eng_t eng;
    int16_t cep;
    uint8_t levels[ASK_MOD_MAX_LEVELS];
    uint8_t payload[3];

    sense_scale_init();
    hrpwm_sr_init(&sr);
    hrpwm_buck_init(&buck);
    protect_init(&prot);
    wpc_sm_rx_init(&sm);

    raw.vrect = 0U;
    raw.vout = 0U;
    raw.iout = 0U;
    raw.ntc_coil = 0U;
    raw.ntc_sr = 0U;
    sense_raw_to_eng(&raw, &eng);

    wpc_sm_rx_on_field(&sm, eng.vrect_mv > VRECT_UV_MV);
    if (protect_eval(&prot, &eng, sm.state != RX_IDLE) != PROT_OK) {
        hrpwm_sr_emb_trip(&sr);
        hrpwm_buck_emb_trip(&buck);
        return -1;
    }

    cep = rx_cep_from_vrect(eng.vrect_mv, VRECT_TGT_MV);
    wpc_sm_rx_update_cep(&sm, cep, eng.prx_mw);

    payload[0] = 0x01;
    payload[1] = 0x80;
    (void)ask_mod_encode(levels, ASK_MOD_MAX_LEVELS, payload, 2U);

    hrpwm_buck_enable(&buck, 1);
    hrpwm_buck_set_duty(&buck, 0.80f);
    (void)k_hrpwm_pins;
    return 0;
}

int main(void)
{
    return wpt_rx_app();
}
#endif
