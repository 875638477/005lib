#include "hrpwm_4ph.h"
#include "app_config.h"
#include "board_pin.h"

/*
 * Target bring-up (DDL-style, not linked on host):
 *
 *   GPIO_SetFunc(PortC, Pin06, Func_Hrpwm1_Pwma);
 *   GPIO_SetFunc(PortC, Pin07, Func_Hrpwm1_Pwmb);
 *   GPIO_SetFunc(PortA, Pin08, Func_Hrpwm2_Pwma);
 *   GPIO_SetFunc(PortA, Pin09, Func_Hrpwm2_Pwmb);
 *   GPIO_SetFunc(PortA, Pin10, Func_Hrpwm3_Pwma);
 *   GPIO_SetFunc(PortA, Pin11, Func_Hrpwm3_Pwmb);
 *   GPIO_SetFunc(PortB, Pin12, Func_Hrpwm4_Pwma);
 *   GPIO_SetFunc(PortB, Pin13, Func_Hrpwm4_Pwmb);
 *
 *   HRPWM unit1 master, triangle, period = HRPWM_PERIOD_TICKS;
 *   unit2/3/4 sync + PHSCMP = 0.25/0.50/0.75 * period;
 *   complementary PWMA/PWMB + deadtime;
 *   EMB from CMP1/CMP2 + PC5.
 */

uint32_t hrpwm_ns_to_ticks(uint32_t ns)
{
    return (uint32_t)(((uint64_t)ns * CPU_CLK_HZ) / 1000000000ULL);
}

void hrpwm_4ph_init(hrpwm_4ph_t *h)
{
    const uint32_t period = HRPWM_PERIOD_TICKS;
    unsigned i;

    h->period = period;
    h->deadtime_ticks = hrpwm_ns_to_ticks(DEADTIME_NS);
    h->phase_ticks[0] = 0U;
    h->phase_ticks[1] = period / 4U;
    h->phase_ticks[2] = period / 2U;
    h->phase_ticks[3] = (period * 3U) / 4U;
    h->enabled = 0U;
    h->emb_latched = 0U;
    for (i = 0; i < PHASE_COUNT; ++i) {
        h->duty[i] = 0.0f;
    }
    (void)k_hrpwm_pins;
}

void hrpwm_4ph_apply_duty(hrpwm_4ph_t *h, const float duty[4])
{
    unsigned i;

    if (h->emb_latched || !h->enabled) {
        for (i = 0; i < PHASE_COUNT; ++i) {
            h->duty[i] = 0.0f;
        }
        return;
    }
    for (i = 0; i < PHASE_COUNT; ++i) {
        h->duty[i] = duty[i];
    }
}

void hrpwm_4ph_enable(hrpwm_4ph_t *h, uint8_t en)
{
    if (h->emb_latched) {
        h->enabled = 0U;
        return;
    }
    h->enabled = en ? 1U : 0U;
}

void hrpwm_4ph_emb_trip(hrpwm_4ph_t *h)
{
    unsigned i;

    h->emb_latched = 1U;
    h->enabled = 0U;
    for (i = 0; i < PHASE_COUNT; ++i) {
        h->duty[i] = 0.0f;
    }
}

void hrpwm_4ph_emb_clear(hrpwm_4ph_t *h)
{
    h->emb_latched = 0U;
}
