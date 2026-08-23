#include "power_ctrl.h"
#include "app_config.h"
#include "wpt_math.h"

#define VBRG_STEP_MV    80
#define FSW_STEP_HZ     400U

void power_ctrl_init(power_ctrl_t *c)
{
    c->vbrg_mv = VBRG_NOM_MV;
    c->vbrg_min_mv = VBRG_MIN_MV;
    c->vbrg_max_mv = VBRG_MAX_MV;
    c->fsw_hz = FSW_INV_HZ;
    c->cep = 0;
    c->vbrg_sat = 0U;
}

void power_ctrl_clamp_for_vin(power_ctrl_t *c, int32_t vin_mv)
{
    int32_t vmax = vin_mv - 400;

    if (vmax > VBRG_MAX_MV) {
        vmax = VBRG_MAX_MV;
    }
    if (vmax < VBRG_MIN_MV) {
        vmax = VBRG_MIN_MV;
    }
    c->vbrg_max_mv = vmax;
    if (c->vbrg_mv > c->vbrg_max_mv) {
        c->vbrg_mv = c->vbrg_max_mv;
    }
}

void power_ctrl_apply_cep(power_ctrl_t *c, int16_t cep)
{
    c->cep = cep;
    c->vbrg_sat = 0U;

    if (cep > 0) {
        c->vbrg_mv += VBRG_STEP_MV * (cep > 3 ? 2 : 1);
        if (c->vbrg_mv >= c->vbrg_max_mv) {
            c->vbrg_mv = c->vbrg_max_mv;
            c->vbrg_sat = 1U;
            if (c->fsw_hz > FSW_INV_MIN_HZ + FSW_STEP_HZ) {
                c->fsw_hz -= FSW_STEP_HZ;
            }
        }
    } else if (cep < 0) {
        c->vbrg_mv -= VBRG_STEP_MV * (cep < -3 ? 2 : 1);
        if (c->vbrg_mv <= c->vbrg_min_mv) {
            c->vbrg_mv = c->vbrg_min_mv;
            c->vbrg_sat = 1U;
            if (c->fsw_hz + FSW_STEP_HZ <= FSW_INV_MAX_HZ) {
                c->fsw_hz += FSW_STEP_HZ;
            }
        }
    }
}

void power_ctrl_to_inv(const power_ctrl_t *c, hrpwm_inv_t *inv)
{
    hrpwm_inv_set_freq(inv, c->fsw_hz);
}
