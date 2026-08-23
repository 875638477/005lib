#include "app_config.h"
#include "board_pin.h"
#include "hrpwm_inv.h"
#include "sense_scale.h"
#include "protect.h"
#include "power_ctrl.h"
#include "wpc_sm.h"
#include "wpt_math.h"
#include "ask_demod.h"

#ifndef HOST_TEST
int wpt_app(void)
{
    hrpwm_inv_t inv;
    protect_t prot;
    power_ctrl_t pwr;
    wpc_sm_t sm;
    ask_demod_t ask;
    sense_raw_t raw;
    sense_eng_t eng;
    int32_t ploss;

    sense_scale_init();
    hrpwm_inv_init(&inv);
    protect_init(&prot);
    power_ctrl_init(&pwr);
    wpc_sm_init(&sm);
    ask_demod_init(&ask);

    raw.vin = 0U;
    raw.vbrg = 0U;
    raw.iin = 0U;
    raw.icoil = 0U;
    raw.ntc_coil = 0U;
    raw.ntc_mos = 0U;
    sense_raw_to_eng(&raw, &eng);

    if (protect_eval(&prot, &eng) != PROT_OK) {
        hrpwm_inv_emb_trip(&inv);
        return -1;
    }

    sm.state = WPC_PING;
    hrpwm_inv_set_mode(&inv, INV_MODE_FULL);
    hrpwm_inv_enable(&inv, 1);
    power_ctrl_clamp_for_vin(&pwr, eng.vin_mv);
    power_ctrl_to_inv(&pwr, &inv);

    ploss = wpt_ploss_mw(eng.ptx_mw, sm.last_prx_mw);
    if (wpt_fod_trip(ploss, FOD_PLOSS_MW)) {
        hrpwm_inv_enable(&inv, 0);
        sm.state = WPC_SEL;
    }

    (void)ask;
    (void)k_hrpwm_pins;
    return 0;
}

int main(void)
{
    return wpt_app();
}
#endif
