#ifndef CONTROL_INV_H
#define CONTROL_INV_H

#include <stdint.h>
#include "app_config.h"
#include "sense_scale.h"
#include "svpwm.h"

typedef struct {
    float kp;
    float ki;
    float i_state;
    float out_min;
    float out_max;
} inv_pi_t;

typedef struct {
    float kp;
    float kr;
    float w0;
    float x1;
    float x2;
} pr_t;

typedef struct {
    uint16_t f_out_hz;
    inv_pwm_mode_t pwm_mode;
    float m;
    float m_ss;
    float theta;
    float v_alpha;
    float vac_rms_v;
    float vac_acc;
    uint32_t rms_n;
    inv_pi_t pi_rms;
    pr_t pr;
    svpwm_t svpwm;
    uint8_t pwm_enable;
} control_inv_t;

void control_inv_init(control_inv_t *c, uint16_t f_out_hz, inv_pwm_mode_t mode);
void control_inv_set_freq(control_inv_t *c, uint16_t f_out_hz);
void control_inv_set_mode(control_inv_t *c, inv_pwm_mode_t mode);
void control_inv_enable(control_inv_t *c, uint8_t en);
void control_inv_step(control_inv_t *c, const sense_eng_t *s, float dt_s);

#endif
