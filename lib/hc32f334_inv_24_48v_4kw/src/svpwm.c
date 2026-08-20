#include "svpwm.h"
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

void svpwm_init(svpwm_t *s, inv_pwm_mode_t mode)
{
    s->mode = mode;
    s->duty_a = 0.5f;
    s->duty_b = 0.5f;
    s->sector = 0U;
    s->v_alpha = 0.0f;
}

void svpwm_set_mode(svpwm_t *s, inv_pwm_mode_t mode)
{
    s->mode = mode;
}

void svpwm_update(svpwm_t *s, float v_alpha, float theta)
{
    float va;
    float t_act;
    float t_z;
    float half;

    va = clampf(v_alpha, -1.0f, 1.0f);
    s->v_alpha = va;

    if (s->mode == INV_PWM_BIPOLAR) {
        s->duty_a = clampf(0.5f + 0.5f * va, 0.0f, 1.0f);
        s->duty_b = s->duty_a;
        s->sector = 0U;
        return;
    }

    if (s->mode == INV_PWM_UNIPOLAR) {
        half = clampf(fabsf(va), 0.0f, 1.0f);
        if (va >= 0.0f) {
            s->duty_a = half;
            s->duty_b = 0.0f;
            s->sector = 1U;
        } else {
            s->duty_a = 0.0f;
            s->duty_b = half;
            s->sector = 2U;
        }
        (void)theta;
        return;
    }

    /* Single-phase 3-level SVPWM: +Vbus / 0 / -Vbus. */
    t_act = fabsf(va);
    t_z = 1.0f - t_act;
    if (va >= 0.0f) {
        s->duty_a = t_z * 0.5f + t_act;
        s->duty_b = t_z * 0.5f;
        s->sector = 1U;
    } else {
        s->duty_a = t_z * 0.5f;
        s->duty_b = t_z * 0.5f + t_act;
        s->sector = 2U;
    }
    s->duty_a = clampf(s->duty_a, 0.0f, 1.0f);
    s->duty_b = clampf(s->duty_b, 0.0f, 1.0f);
    (void)theta;
}
