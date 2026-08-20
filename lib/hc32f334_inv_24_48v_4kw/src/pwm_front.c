#include "pwm_front.h"
#include "app_config.h"
#include "board_pin.h"

uint32_t pwm_ns_to_ticks(uint32_t ns)
{
    return (uint32_t)(((uint64_t)ns * CPU_CLK_HZ) / 1000000000ULL);
}

void pwm_front_init(pwm_front_t *h)
{
    const uint32_t period = DCDC_PERIOD_TICKS;
    unsigned i;

    h->period = period;
    h->deadtime_ticks = pwm_ns_to_ticks(DCDC_DEADTIME_NS);
    h->carrier_ticks[0] = 0U;
    h->carrier_ticks[1] = period / 4U;
    h->carrier_ticks[2] = period / 2U;
    h->carrier_ticks[3] = (period * 3U) / 4U;
    h->enabled = 0U;
    h->emb_latched = 0U;
    for (i = 0; i < PHASE_COUNT; ++i) {
        h->deff[i] = 0.0f;
        h->phase_ticks[i] = period / 2U;
    }
    (void)k_front_a_pins;
    (void)k_front_b_pins;
}

static float clampf(float x, float lo, float hi)
{
    if (x < lo) {
        return lo;
    }
    if (x > hi) {
        return hi;
    }
    return x;
}

void pwm_front_apply_deff(pwm_front_t *h, const float deff[4])
{
    unsigned i;

    if (h->emb_latched || !h->enabled) {
        for (i = 0; i < PHASE_COUNT; ++i) {
            h->deff[i] = 0.0f;
            h->phase_ticks[i] = h->period / 2U;
        }
        return;
    }
    for (i = 0; i < PHASE_COUNT; ++i) {
        float d = clampf(deff[i], 0.0f, DEFF_MAX);
        h->deff[i] = d;
        /* A-leg 50%. B-leg delayed so diagonal overlap = Deff * 180 deg. */
        h->phase_ticks[i] = (uint32_t)((1.0f - d) * (float)(h->period / 2U) + 0.5f);
    }
}

void pwm_front_apply_common(pwm_front_t *h, float deff)
{
    float d[4];
    unsigned i;

    for (i = 0; i < PHASE_COUNT; ++i) {
        d[i] = deff;
    }
    pwm_front_apply_deff(h, d);
}

void pwm_front_enable(pwm_front_t *h, uint8_t en)
{
    if (h->emb_latched) {
        h->enabled = 0U;
        return;
    }
    h->enabled = en ? 1U : 0U;
}

void pwm_front_emb_trip(pwm_front_t *h)
{
    unsigned i;

    h->emb_latched = 1U;
    h->enabled = 0U;
    for (i = 0; i < PHASE_COUNT; ++i) {
        h->deff[i] = 0.0f;
        h->phase_ticks[i] = h->period / 2U;
    }
}

void pwm_front_emb_clear(pwm_front_t *h)
{
    h->emb_latched = 0U;
}

uint32_t pwm_front_lag_ticks(const pwm_front_t *h, unsigned ph)
{
    if (ph >= PHASE_COUNT) {
        return 0U;
    }
    return (h->carrier_ticks[ph] + h->phase_ticks[ph]) % h->period;
}
