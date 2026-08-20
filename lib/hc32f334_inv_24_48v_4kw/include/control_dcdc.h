#ifndef CONTROL_DCDC_H
#define CONTROL_DCDC_H

#include <stdint.h>
#include "sense_scale.h"

typedef struct {
    float kp;
    float ki;
    float i_state;
    float out_min;
    float out_max;
} pi_t;

typedef struct {
    int32_t vref_mv;
    int32_t vref_ss_mv;
    float deff;
    float deff_ph[4];
    pi_t pi_v;
    uint8_t pwm_enable;
} control_dcdc_t;

void control_dcdc_init(control_dcdc_t *c);
void control_dcdc_enable(control_dcdc_t *c, uint8_t en);
void control_dcdc_set_vref(control_dcdc_t *c, int32_t vref_mv);
void control_dcdc_step(control_dcdc_t *c, const sense_eng_t *s, float dt_s);
float control_dcdc_feedforward(int32_t vin_mv, int32_t vbus_mv);

#endif
