#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <stdint.h>
#include "sense_scale.h"

typedef enum {
    CTRL_MODE_VOLTAGE = 0,
    CTRL_MODE_CURRENT
} ctrl_mode_t;

typedef enum {
    CTRL_REG_CV = 0,
    CTRL_REG_CC
} ctrl_reg_t;

typedef struct {
    float kp;
    float ki;
    float i_state;
    float out_min;
    float out_max;
} pi_t;

typedef struct {
    ctrl_mode_t mode;
    ctrl_reg_t  reg;
    int32_t vref_mv;
    int32_t iset_ma;
    int32_t vref_ss_mv;
    float duty[4];
    float iref_tot_a;
    float iref_ph_a[4];
    pi_t  pi_v;
    pi_t  pi_i[4];
    pi_t  pi_share;
    uint8_t pwm_enable;
    uint32_t ss_ticks;
} control_t;

void control_init(control_t *c, ctrl_mode_t mode);
void control_set_mode(control_t *c, ctrl_mode_t mode);
void control_set_cv_cc(control_t *c, int32_t vref_mv, int32_t iset_ma);
void control_enable(control_t *c, uint8_t en);
void control_reset_integrators(control_t *c);
void control_step(control_t *c, const sense_eng_t *s, float dt_s);
float control_clamp_duty(float d);

#endif
