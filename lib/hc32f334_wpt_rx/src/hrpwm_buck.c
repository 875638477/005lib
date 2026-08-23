#include "hrpwm_buck.h"
#include "app_config.h"

void hrpwm_buck_init(hrpwm_buck_t *h)
{
    h->fsw_hz = FSW_BUCK_HZ;
    h->period_ticks = CPU_CLK_HZ / FSW_BUCK_HZ;
    h->duty = 0.0f;
    h->enabled = 0U;
    h->emb_latched = 0U;
}

void hrpwm_buck_set_duty(hrpwm_buck_t *h, float duty)
{
    if (duty < 0.02f) {
        duty = 0.02f;
    }
    if (duty > 0.92f) {
        duty = 0.92f;
    }
    if (h->emb_latched || !h->enabled) {
        h->duty = 0.0f;
        return;
    }
    h->duty = duty;
}

void hrpwm_buck_enable(hrpwm_buck_t *h, uint8_t en)
{
    if (h->emb_latched) {
        h->enabled = 0U;
        h->duty = 0.0f;
        return;
    }
    h->enabled = en ? 1U : 0U;
    if (!h->enabled) {
        h->duty = 0.0f;
    }
}

void hrpwm_buck_emb_trip(hrpwm_buck_t *h)
{
    h->emb_latched = 1U;
    h->enabled = 0U;
    h->duty = 0.0f;
}
