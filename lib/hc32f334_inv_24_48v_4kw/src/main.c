#include "app_config.h"
#include "board_pin.h"
#include "adc_acq.h"
#include "protect.h"
#include "control_dcdc.h"
#include "control_inv.h"
#include "pwm_front.h"
#include "pwm_inv.h"

#ifndef HOST_TEST
int inverter_app(void)
{
    control_dcdc_t dcdc;
    control_inv_t inv;
    protect_t prot;
    pwm_front_t pwm_fb;
    pwm_inv_t pwm_ac;
    sense_raw_t raw;
    sense_eng_t eng;

    sense_scale_init();
    adc_acq_init();
    pwm_front_init(&pwm_fb);
    pwm_inv_init(&pwm_ac);
    protect_init(&prot);
    control_dcdc_init(&dcdc);
    control_inv_init(&inv, F_OUT_DEFAULT_HZ, INV_PWM_SVPWM);

    adc_acq_read(&raw);
    sense_raw_to_eng(&raw, &eng);

    if (protect_eval(&prot, &eng, 250U) != PROT_OK) {
        pwm_front_emb_trip(&pwm_fb);
        pwm_inv_emb_trip(&pwm_ac);
        control_dcdc_enable(&dcdc, 0);
        control_inv_enable(&inv, 0);
        return -1;
    }

    control_dcdc_enable(&dcdc, 1);
    pwm_front_enable(&pwm_fb, 1);
    control_dcdc_step(&dcdc, &eng, 1.0f / (float)CTRL_DCDC_HZ);
    pwm_front_apply_common(&pwm_fb, dcdc.deff);

    if (prot.state == PROT_ST_INV_SS || prot.state == PROT_ST_RUN) {
        control_inv_enable(&inv, 1);
        pwm_inv_enable(&pwm_ac, 1);
        control_inv_step(&inv, &eng, 1.0f / (float)CTRL_INV_HZ);
        pwm_inv_apply(&pwm_ac, &inv.svpwm);
    }

    (void)k_front_a_pins;
    return 0;
}

int main(void)
{
    return inverter_app();
}
#endif
