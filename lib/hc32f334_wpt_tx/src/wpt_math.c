#include "wpt_math.h"
#include "app_config.h"

#include <math.h>

uint32_t wpt_fres_hz(uint32_t ltx_nh, uint32_t cs_nf)
{
    double l = (double)ltx_nh * 1e-9;
    double c = (double)cs_nf * 1e-9;
    double lc;

    if (l <= 0.0 || c <= 0.0) {
        return 0U;
    }
    lc = l * c;
    return (uint32_t)(1.0 / (2.0 * 3.141592653589793 * sqrt(lc)) + 0.5);
}

uint32_t wpt_hrpwm_period_ticks(uint32_t fsw_hz)
{
    if (fsw_hz < FSW_INV_MIN_HZ) {
        fsw_hz = FSW_INV_MIN_HZ;
    }
    if (fsw_hz > FSW_INV_MAX_HZ) {
        fsw_hz = FSW_INV_MAX_HZ;
    }
    return CPU_CLK_HZ / fsw_hz;
}

uint32_t wpt_fsk_delta_ticks(uint32_t fsw_hz, uint32_t depth_hz)
{
    uint32_t p0 = wpt_hrpwm_period_ticks(fsw_hz);
    uint32_t p1;
    uint32_t f1;

    if (depth_hz == 0U) {
        return 0U;
    }
    f1 = fsw_hz + depth_hz;
    p1 = wpt_hrpwm_period_ticks(f1);
    return (p0 > p1) ? (p0 - p1) : (p1 - p0);
}

int32_t wpt_pin_cap_mw(int32_t vin_mv)
{
    int32_t cap = P_IN_CAP_MW;

    if (vin_mv < 11000) {
        cap = (20000 * 110) / 80;
    } else if (vin_mv < 14000) {
        cap = (30000 * 110) / 80;
    }
    if (cap > P_IN_CAP_MW) {
        cap = P_IN_CAP_MW;
    }
    return cap;
}

int32_t wpt_contract_mw_for_vin(int32_t vin_mv)
{
    int32_t c = P_CONTRACT_MW;

    if (vin_mv < 11000) {
        c = 20000;
    } else if (vin_mv < 14000) {
        c = 30000;
    }
    if (c > P_CONTRACT_MW) {
        c = P_CONTRACT_MW;
    }
    return c;
}

int32_t wpt_ploss_mw(int32_t ptx_mw, int32_t prx_mw)
{
    return ptx_mw - prx_mw;
}

int wpt_fod_trip(int32_t ploss_mw, int32_t threshold_mw)
{
    return ploss_mw > threshold_mw;
}

float wpt_q_from_decay(float a0, float an, unsigned n_cycles)
{
    float ratio;

    if (a0 <= 0.0f || an <= 0.0f || n_cycles == 0U || an >= a0) {
        return 0.0f;
    }
    ratio = a0 / an;
    return (3.14159265f * (float)n_cycles) / logf(ratio);
}
