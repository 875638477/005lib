#ifndef HRPWM_BUCK_H
#define HRPWM_BUCK_H

#include <stdint.h>

typedef struct {
    uint32_t fsw_hz;
    uint32_t period_ticks;
    float duty;
    uint8_t enabled;
    uint8_t emb_latched;
} hrpwm_buck_t;

void hrpwm_buck_init(hrpwm_buck_t *h);
void hrpwm_buck_set_duty(hrpwm_buck_t *h, float duty);
void hrpwm_buck_enable(hrpwm_buck_t *h, uint8_t en);
void hrpwm_buck_emb_trip(hrpwm_buck_t *h);

#endif
