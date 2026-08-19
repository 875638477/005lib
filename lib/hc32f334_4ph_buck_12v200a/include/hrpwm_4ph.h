#ifndef HRPWM_4PH_H
#define HRPWM_4PH_H

#include <stdint.h>

typedef struct {
    uint32_t period;
    uint32_t deadtime_ticks;
    uint32_t phase_ticks[4];
    float duty[4];
    uint8_t enabled;
    uint8_t emb_latched;
} hrpwm_4ph_t;

void hrpwm_4ph_init(hrpwm_4ph_t *h);
void hrpwm_4ph_apply_duty(hrpwm_4ph_t *h, const float duty[4]);
void hrpwm_4ph_enable(hrpwm_4ph_t *h, uint8_t en);
void hrpwm_4ph_emb_trip(hrpwm_4ph_t *h);
void hrpwm_4ph_emb_clear(hrpwm_4ph_t *h);
uint32_t hrpwm_ns_to_ticks(uint32_t ns);

#endif
