#ifndef PWM_INV_H
#define PWM_INV_H

#include <stdint.h>
#include "svpwm.h"

typedef struct {
    uint32_t period;
    uint32_t deadtime_ticks;
    float duty_a;
    float duty_b;
    uint8_t enabled;
    uint8_t emb_latched;
} pwm_inv_t;

void pwm_inv_init(pwm_inv_t *h);
void pwm_inv_apply(pwm_inv_t *h, const svpwm_t *s);
void pwm_inv_enable(pwm_inv_t *h, uint8_t en);
void pwm_inv_emb_trip(pwm_inv_t *h);
void pwm_inv_emb_clear(pwm_inv_t *h);
uint32_t pwm_inv_cmp_a(const pwm_inv_t *h);
uint32_t pwm_inv_cmp_b(const pwm_inv_t *h);

#endif
