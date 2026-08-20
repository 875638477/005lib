#include "control_dcdc.h"
#include "app_config.h"

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

static float pi_step(pi_t *p, float err, float dt)
{
    float out;

    p->i_state += p->ki * err * dt;
    p->i_state = clampf(p->i_state, p->out_min, p->out_max);
    out = p->kp * err + p->i_state;
    return clampf(out, p->out_min, p->out_max);
}

float control_dcdc_feedforward(int32_t vin_mv, int32_t vbus_mv)
{
    float vin;
    float vmod;
    float d;

    vin = (float)vin_mv / 1000.0f;
    if (vin < 1.0f) {
        return DEFF_MIN;
    }
    vmod = (float)vbus_mv / (1000.0f * (float)PHASE_COUNT);
    d = vmod / (XFMR_RATIO * vin);
    return clampf(d, DEFF_MIN, DEFF_MAX);
}

void control_dcdc_init(control_dcdc_t *c)
{
    unsigned i;

    c->vref_mv = VBUS_NOM_MV;
    c->vref_ss_mv = 0;
    c->deff = 0.0f;
    c->pwm_enable = 0U;
    c->pi_v.kp = 0.0008f;
    c->pi_v.ki = 0.012f;
    c->pi_v.i_state = 0.0f;
    c->pi_v.out_min = -0.15f;
    c->pi_v.out_max = 0.15f;
    for (i = 0; i < PHASE_COUNT; ++i) {
        c->deff_ph[i] = 0.0f;
    }
}

void control_dcdc_enable(control_dcdc_t *c, uint8_t en)
{
    c->pwm_enable = en ? 1U : 0U;
    if (!en) {
        unsigned i;
        c->deff = 0.0f;
        c->vref_ss_mv = 0;
        c->pi_v.i_state = 0.0f;
        for (i = 0; i < PHASE_COUNT; ++i) {
            c->deff_ph[i] = 0.0f;
        }
    }
}

void control_dcdc_set_vref(control_dcdc_t *c, int32_t vref_mv)
{
    if (vref_mv < 0) {
        vref_mv = 0;
    }
    if (vref_mv > VBUS_OV_SOFT_MV) {
        vref_mv = VBUS_OV_SOFT_MV;
    }
    c->vref_mv = vref_mv;
}

void control_dcdc_step(control_dcdc_t *c, const sense_eng_t *s, float dt_s)
{
    float ff;
    float corr;
    float dmax;
    int32_t ss_step;
    unsigned i;

    if (!c->pwm_enable) {
        c->deff = 0.0f;
        for (i = 0; i < PHASE_COUNT; ++i) {
            c->deff_ph[i] = 0.0f;
        }
        return;
    }

    ss_step = (int32_t)(SOFTSTART_VBUS_V_PER_S * dt_s * 1000.0f);
    if (ss_step < 1) {
        ss_step = 1;
    }
    if (c->vref_ss_mv < c->vref_mv) {
        c->vref_ss_mv += ss_step;
        if (c->vref_ss_mv > c->vref_mv) {
            c->vref_ss_mv = c->vref_mv;
        }
    } else {
        c->vref_ss_mv = c->vref_mv;
    }

    dmax = DEFF_MAX;
    if (c->vref_ss_mv < c->vref_mv && dmax > DEFF_SOFTSTART_MAX) {
        dmax = DEFF_SOFTSTART_MAX;
    }

    ff = control_dcdc_feedforward(s->vin_mv, c->vref_ss_mv);
    corr = pi_step(&c->pi_v,
                   ((float)c->vref_ss_mv - (float)s->vbus_mv) / 1000.0f,
                   dt_s);
    c->deff = clampf(ff + corr, DEFF_MIN, dmax);

    for (i = 0; i < PHASE_COUNT; ++i) {
        c->deff_ph[i] = c->deff;
    }
}
