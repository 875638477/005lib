#ifndef PWM_FRONT_H
#define PWM_FRONT_H

#include <stdint.h>

typedef struct {
    uint32_t period;
    uint32_t deadtime_ticks;
    uint32_t carrier_ticks[4];
    uint32_t phase_ticks[4];
    float deff[4];
    uint8_t enabled;
    uint8_t emb_latched;
} pwm_front_t;

void pwm_front_init(pwm_front_t *h);
void pwm_front_apply_deff(pwm_front_t *h, const float deff[4]);
void pwm_front_apply_common(pwm_front_t *h, float deff);
void pwm_front_enable(pwm_front_t *h, uint8_t en);
void pwm_front_emb_trip(pwm_front_t *h);
void pwm_front_emb_clear(pwm_front_t *h);
uint32_t pwm_ns_to_ticks(uint32_t ns);
uint32_t pwm_front_lag_ticks(const pwm_front_t *h, unsigned ph);

#endif
