#include "control_inv.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static float pi_step(inv_pi_t *p, float err, float dt)
{
    float out;

    p->i_state += p->ki * err * dt;
    p->i_state = clampf(p->i_state, p->out_min, p->out_max);
    out = p->kp * err + p->i_state;
    return clampf(out, p->out_min, p->out_max);
}

static float pr_step(pr_t *p, float err, float dt)
{
    float x1n;
    float y;

    /* Resonator: x' = w0*(-x2 + err*kr? handled outside), bilinear-ish Euler */
    x1n = p->x1 + dt * (p->w0 * p->x2 + err);
    p->x2 = p->x2 - dt * (p->w0 * p->x1);
    p->x1 = x1n;
    y = p->kp * err + p->kr * p->x1;
    return y;
}

void control_inv_init(control_inv_t *c, uint16_t f_out_hz, inv_pwm_mode_t mode)
{
    c->f_out_hz = (f_out_hz == F_OUT_60_HZ) ? F_OUT_60_HZ : F_OUT_50_HZ;
    c->pwm_mode = mode;
    c->m = 0.0f;
    c->m_ss = 0.0f;
    c->theta = 0.0f;
    c->v_alpha = 0.0f;
    c->vac_rms_v = 0.0f;
    c->vac_acc = 0.0f;
    c->rms_n = 0U;
    c->pwm_enable = 0U;
    c->pi_rms.kp = 0.002f;
    c->pi_rms.ki = 0.08f;
    c->pi_rms.i_state = 0.0f;
    c->pi_rms.out_min = -0.20f;
    c->pi_rms.out_max = 0.20f;
    c->pr.kp = 0.0004f;
    c->pr.kr = 8.0f;
    c->pr.w0 = 2.0f * (float)M_PI * (float)c->f_out_hz;
    c->pr.x1 = 0.0f;
    c->pr.x2 = 0.0f;
    svpwm_init(&c->svpwm, mode);
}

void control_inv_set_freq(control_inv_t *c, uint16_t f_out_hz)
{
    c->f_out_hz = (f_out_hz == F_OUT_60_HZ) ? F_OUT_60_HZ : F_OUT_50_HZ;
    c->pr.w0 = 2.0f * (float)M_PI * (float)c->f_out_hz;
}

void control_inv_set_mode(control_inv_t *c, inv_pwm_mode_t mode)
{
    c->pwm_mode = mode;
    svpwm_set_mode(&c->svpwm, mode);
}

void control_inv_enable(control_inv_t *c, uint8_t en)
{
    c->pwm_enable = en ? 1U : 0U;
    if (!en) {
        c->m = 0.0f;
        c->m_ss = 0.0f;
        c->theta = 0.0f;
        c->v_alpha = 0.0f;
        c->vac_acc = 0.0f;
        c->rms_n = 0U;
        c->pi_rms.i_state = 0.0f;
        c->pr.x1 = 0.0f;
        c->pr.x2 = 0.0f;
        svpwm_init(&c->svpwm, c->pwm_mode);
    }
}

void control_inv_step(control_inv_t *c, const sense_eng_t *s, float dt_s)
{
    float w;
    float vbus;
    float vref;
    float vac;
    float m_ff;
    float m_corr;
    float pr_out;
    float two_pi;

    if (!c->pwm_enable) {
        c->v_alpha = 0.0f;
        svpwm_update(&c->svpwm, 0.0f, c->theta);
        return;
    }

    two_pi = 2.0f * (float)M_PI;
    w = two_pi * (float)c->f_out_hz;
    c->theta += w * dt_s;
    if (c->theta >= two_pi) {
        c->theta -= two_pi;
    }

    c->m_ss += SOFTSTART_M_PER_S * dt_s;
    if (c->m_ss > M_MAX) {
        c->m_ss = M_MAX;
    }

    vbus = (float)s->vbus_mv / 1000.0f;
    if (vbus < 50.0f) {
        vbus = 50.0f;
    }
    m_ff = ((float)VAC_PEAK_NOM_MV / 1000.0f) / vbus;
    m_ff = clampf(m_ff, M_MIN, M_MAX);

    vac = (float)s->vac_mv / 1000.0f;
    c->vac_acc += vac * vac;
    c->rms_n++;
    if (c->rms_n >= (FSW_INV_HZ / (uint32_t)c->f_out_hz)) {
        c->vac_rms_v = sqrtf(c->vac_acc / (float)c->rms_n);
        c->vac_acc = 0.0f;
        c->rms_n = 0U;
        m_corr = pi_step(&c->pi_rms,
                         ((float)VAC_RMS_NOM_MV / 1000.0f) - c->vac_rms_v,
                         1.0f / (float)c->f_out_hz);
        c->m = clampf(m_ff + m_corr, M_MIN, c->m_ss);
    } else if (c->m < M_MIN) {
        c->m = clampf(m_ff, M_MIN, c->m_ss);
    }

    vref = c->m * sinf(c->theta) * vbus;
    pr_out = pr_step(&c->pr, vref - vac, dt_s);
    c->v_alpha = clampf((vref + pr_out) / vbus, -c->m_ss, c->m_ss);
    svpwm_update(&c->svpwm, c->v_alpha, c->theta);
}
