#include "control_loop.h"
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

float control_clamp_duty(float d)
{
    return clampf(d, DMIN_RUN, DMAX_RUN);
}

void control_reset_integrators(control_t *c)
{
    unsigned i;

    c->pi_v.i_state = 0.0f;
    c->pi_share.i_state = 0.0f;
    for (i = 0; i < PHASE_COUNT; ++i) {
        c->pi_i[i].i_state = 0.0f;
        c->duty[i] = 0.0f;
        c->iref_ph_a[i] = 0.0f;
    }
    c->iref_tot_a = 0.0f;
}

void control_init(control_t *c, ctrl_mode_t mode)
{
    unsigned i;

    c->mode = mode;
    c->reg = CTRL_REG_CV;
    c->vref_mv = VOUT_NOM_MV;
    c->iset_ma = IOUT_RATED_MA;
    c->vref_ss_mv = 0;
    c->pwm_enable = 0U;
    c->ss_ticks = 0U;

    c->pi_v.kp = 40.0f;
    c->pi_v.ki = 8000.0f;
    c->pi_v.out_min = 0.0f;
    c->pi_v.out_max = (float)IOUT_RATED_MA / 1000.0f;

    c->pi_share.kp = 0.15f;
    c->pi_share.ki = 8.0f;
    c->pi_share.out_min = -15.0f;
    c->pi_share.out_max = 15.0f;

    for (i = 0; i < PHASE_COUNT; ++i) {
        c->pi_i[i].kp = 0.012f;
        c->pi_i[i].ki = 25.0f;
        c->pi_i[i].out_min = DMIN_RUN;
        c->pi_i[i].out_max = DMAX_SOFTSTART;
    }
    control_reset_integrators(c);
}

void control_set_mode(control_t *c, ctrl_mode_t mode)
{
    c->mode = mode;
    control_reset_integrators(c);
}

void control_set_cv_cc(control_t *c, int32_t vref_mv, int32_t iset_ma)
{
    if (vref_mv < 0) {
        vref_mv = 0;
    }
    if (vref_mv > OVP_SOFT_MV) {
        vref_mv = OVP_SOFT_MV;
    }
    if (iset_ma < 0) {
        iset_ma = 0;
    }
    if (iset_ma > IOUT_RATED_MA) {
        iset_ma = IOUT_RATED_MA;
    }
    c->vref_mv = vref_mv;
    c->iset_ma = iset_ma;
}

void control_enable(control_t *c, uint8_t en)
{
    c->pwm_enable = en ? 1U : 0U;
    if (!en) {
        control_reset_integrators(c);
        c->vref_ss_mv = 0;
        c->ss_ticks = 0U;
    }
}

static float vin_dmax(int32_t vin_mv)
{
    float d;

    if (vin_mv < VIN_UVLO_MV) {
        return DMIN_RUN;
    }
    d = (float)VOUT_NOM_MV / (float)vin_mv;
    d += 0.08f;
    return clampf(d, DMIN_RUN, DMAX_RUN);
}

void control_step(control_t *c, const sense_eng_t *s, float dt_s)
{
    unsigned i;
    float vref;
    float err_v;
    float iref;
    float i_avg;
    float dmax;
    float share;
    int32_t ss_step;

    if (!c->pwm_enable) {
        for (i = 0; i < PHASE_COUNT; ++i) {
            c->duty[i] = 0.0f;
        }
        return;
    }

    ss_step = (int32_t)(SOFTSTART_V_PER_S * dt_s * 1000.0f);
    if (ss_step < 1) {
        ss_step = 1;
    }
    if (s->vout_mv > c->vref_ss_mv && s->vout_mv < c->vref_mv) {
        c->vref_ss_mv = s->vout_mv;
    }
    if (c->vref_ss_mv < c->vref_mv) {
        c->vref_ss_mv += ss_step;
        if (c->vref_ss_mv > c->vref_mv) {
            c->vref_ss_mv = c->vref_mv;
        }
    } else {
        c->vref_ss_mv = c->vref_mv;
    }

    vref = (float)c->vref_ss_mv / 1000.0f;
    err_v = vref - ((float)s->vout_mv / 1000.0f);
    iref = pi_step(&c->pi_v, err_v, dt_s);

    if (iref >= ((float)c->iset_ma / 1000.0f) - 0.5f) {
        c->reg = CTRL_REG_CC;
        iref = (float)c->iset_ma / 1000.0f;
        c->pi_v.i_state = iref;
    } else {
        c->reg = CTRL_REG_CV;
    }

    c->iref_tot_a = iref;
    i_avg = 0.0f;
    for (i = 0; i < PHASE_COUNT; ++i) {
        i_avg += (float)s->iph_ma[i] / 1000.0f;
    }
    i_avg *= 0.25f;

    dmax = vin_dmax(s->vin_mv);
    if (c->vref_ss_mv < c->vref_mv) {
        if (dmax > DMAX_SOFTSTART) {
            dmax = DMAX_SOFTSTART;
        }
    }

    for (i = 0; i < PHASE_COUNT; ++i) {
        float iph = (float)s->iph_ma[i] / 1000.0f;
        float corr = pi_step(&c->pi_share, i_avg - iph, dt_s);

        c->iref_ph_a[i] = (c->iref_tot_a * 0.25f) + corr;
        if (c->iref_ph_a[i] < 0.0f) {
            c->iref_ph_a[i] = 0.0f;
        }

        c->pi_i[i].out_max = dmax;

        if (c->mode == CTRL_MODE_CURRENT) {
            c->duty[i] = pi_step(&c->pi_i[i], c->iref_ph_a[i] - iph, dt_s);
        } else {
            share = 0.002f * (i_avg - iph);
            c->duty[i] = clampf((c->iref_tot_a / ((float)IOUT_RATED_MA / 1000.0f)) * dmax + share,
                                DMIN_RUN, dmax);
        }
        c->duty[i] = clampf(c->duty[i], DMIN_RUN, dmax);
    }

    (void)c->ss_ticks;
    c->ss_ticks++;
}
