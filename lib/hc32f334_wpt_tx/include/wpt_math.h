#ifndef WPT_MATH_H
#define WPT_MATH_H

#include <stdint.h>

uint32_t wpt_fres_hz(uint32_t ltx_nh, uint32_t cs_nf);
uint32_t wpt_hrpwm_period_ticks(uint32_t fsw_hz);
uint32_t wpt_fsk_delta_ticks(uint32_t fsw_hz, uint32_t depth_hz);
int32_t wpt_pin_cap_mw(int32_t vin_mv);
int32_t wpt_contract_mw_for_vin(int32_t vin_mv);
int32_t wpt_ploss_mw(int32_t ptx_mw, int32_t prx_mw);
int wpt_fod_trip(int32_t ploss_mw, int32_t threshold_mw);
float wpt_q_from_decay(float a0, float an, unsigned n_cycles);

#endif
