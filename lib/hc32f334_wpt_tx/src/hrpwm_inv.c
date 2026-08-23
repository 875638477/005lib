#include "hrpwm_inv.h"
#include "app_config.h"
#include "board_pin.h"
#include "wpt_math.h"

/*
 * Target bring-up (DDL-style, not linked on host):
 *
 *   GPIO_SetFunc(PortC, Pin06, Func_Hrpwm1_Pwma);
 *   GPIO_SetFunc(PortC, Pin07, Func_Hrpwm1_Pwmb);
 *   GPIO_SetFunc(PortA, Pin08, Func_Hrpwm2_Pwma);
 *   GPIO_SetFunc(PortA, Pin09, Func_Hrpwm2_Pwmb);
 *   GPIO_SetFunc(PortA, Pin10, Func_Hrpwm3_Pwma);
 *   GPIO_SetFunc(PortA, Pin11, Func_Hrpwm3_Pwmb);
 *
 *   HRPWM unit1 master, sawtooth, period = ticks(fsw);
 *   unit2 sync, PHSCMP = phase_deg/360 * period;
 *   complementary PWMA/PWMB + deadtime;
 *   half-bridge: unit2 PWMA=0, PWMB=1 (下管常开);
 *   EMB from CMP1 + PC5.
 */

uint32_t hrpwm_ns_to_ticks(uint32_t ns)
{
    return (uint32_t)(((uint64_t)ns * CPU_CLK_HZ) / 1000000000ULL);
}

static void clamp_freq(hrpwm_inv_t *h)
{
    if (h->fsw_hz < FSW_INV_MIN_HZ) {
        h->fsw_hz = FSW_INV_MIN_HZ;
    }
    if (h->fsw_hz > FSW_INV_MAX_HZ) {
        h->fsw_hz = FSW_INV_MAX_HZ;
    }
    h->period_ticks = wpt_hrpwm_period_ticks(h->fsw_hz);
}

void hrpwm_inv_init(hrpwm_inv_t *h)
{
    h->fsw_hz = FSW_INV_HZ;
    h->deadtime_ticks = hrpwm_ns_to_ticks(DEADTIME_NS);
    h->phase_deg = 180U;
    h->duty = 0.50f;
    h->mode = INV_MODE_FULL;
    h->enabled = 0U;
    h->emb_latched = 0U;
    clamp_freq(h);
    (void)k_hrpwm_pins;
}

void hrpwm_inv_set_freq(hrpwm_inv_t *h, uint32_t fsw_hz)
{
    if (h->emb_latched) {
        return;
    }
    h->fsw_hz = fsw_hz;
    clamp_freq(h);
}

void hrpwm_inv_set_phase(hrpwm_inv_t *h, uint32_t phase_deg)
{
    if (phase_deg > 180U) {
        phase_deg = 180U;
    }
    h->phase_deg = phase_deg;
}

void hrpwm_inv_set_duty(hrpwm_inv_t *h, float duty)
{
    if (duty < 0.10f) {
        duty = 0.10f;
    }
    if (duty > 0.50f) {
        duty = 0.50f;
    }
    h->duty = duty;
}

void hrpwm_inv_set_mode(hrpwm_inv_t *h, inv_mode_t mode)
{
    h->mode = mode;
}

void hrpwm_inv_enable(hrpwm_inv_t *h, uint8_t en)
{
    if (h->emb_latched) {
        h->enabled = 0U;
        return;
    }
    h->enabled = en ? 1U : 0U;
}

void hrpwm_inv_emb_trip(hrpwm_inv_t *h)
{
    h->emb_latched = 1U;
    h->enabled = 0U;
}

void hrpwm_inv_emb_clear(hrpwm_inv_t *h)
{
    h->emb_latched = 0U;
}
