#include "pwm_inv.h"
#include "app_config.h"
#include "board_pin.h"
#include "pwm_front.h"

void pwm_inv_init(pwm_inv_t *h)
{
    h->period = INV_PERIOD_TICKS;
    h->deadtime_ticks = pwm_ns_to_ticks(INV_DEADTIME_NS);
    h->duty_a = 0.5f;
    h->duty_b = 0.5f;
    h->enabled = 0U;
    h->emb_latched = 0U;
    (void)k_inv_pins;
}

void pwm_inv_apply(pwm_inv_t *h, const svpwm_t *s)
{
    if (h->emb_latched || !h->enabled) {
        h->duty_a = 0.5f;
        h->duty_b = 0.5f;
        return;
    }
    h->duty_a = s->duty_a;
    h->duty_b = s->duty_b;
}

void pwm_inv_enable(pwm_inv_t *h, uint8_t en)
{
    if (h->emb_latched) {
        h->enabled = 0U;
        return;
    }
    h->enabled = en ? 1U : 0U;
}

void pwm_inv_emb_trip(pwm_inv_t *h)
{
    h->emb_latched = 1U;
    h->enabled = 0U;
    h->duty_a = 0.5f;
    h->duty_b = 0.5f;
}

void pwm_inv_emb_clear(pwm_inv_t *h)
{
    h->emb_latched = 0U;
}

uint32_t pwm_inv_cmp_a(const pwm_inv_t *h)
{
    return (uint32_t)(h->duty_a * (float)h->period + 0.5f);
}

uint32_t pwm_inv_cmp_b(const pwm_inv_t *h)
{
    return (uint32_t)(h->duty_b * (float)h->period + 0.5f);
}
