#ifndef SVPWM_H
#define SVPWM_H

#include <stdint.h>
#include "app_config.h"

typedef struct {
    inv_pwm_mode_t mode;
    float duty_a;
    float duty_b;
    uint8_t sector;
    float v_alpha;
} svpwm_t;

void svpwm_init(svpwm_t *s, inv_pwm_mode_t mode);
void svpwm_set_mode(svpwm_t *s, inv_pwm_mode_t mode);

/*
 * v_alpha = m * sin(theta), typically in [-M_MAX, M_MAX].
 * theta in radians. polarity is sign of the AC half-cycle (for unipolar).
 */
void svpwm_update(svpwm_t *s, float v_alpha, float theta);

#endif
