#include "app_config.h"
#include "board_pin.h"
#include "control_loop.h"
#include "protect.h"
#include "hrpwm_4ph.h"
#include "adc_acq.h"

#ifndef HOST_TEST
int converter_app(void)
{
    control_t ctrl;
    protect_t prot;
    hrpwm_4ph_t pwm;
    sense_raw_t raw;
    sense_eng_t eng;

    sense_scale_init();
    adc_acq_init();
    hrpwm_4ph_init(&pwm);
    protect_init(&prot);
    control_init(&ctrl, CTRL_MODE_CURRENT);
    control_set_cv_cc(&ctrl, VOUT_NOM_MV, IOUT_RATED_MA);

    adc_acq_read(&raw);
    sense_raw_to_eng(&raw, &eng);

    if (protect_eval(&prot, &eng, 20U) != PROT_OK) {
        hrpwm_4ph_emb_trip(&pwm);
        control_enable(&ctrl, 0);
        return -1;
    }

    control_enable(&ctrl, 1);
    hrpwm_4ph_enable(&pwm, 1);
    control_step(&ctrl, &eng, 1.0f / (float)CTRL_CM_HZ);
    hrpwm_4ph_apply_duty(&pwm, ctrl.duty);
    (void)k_hrpwm_pins;
    return 0;
}

int main(void)
{
    return converter_app();
}
#endif
