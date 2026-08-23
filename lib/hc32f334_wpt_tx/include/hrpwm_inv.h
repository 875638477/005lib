#ifndef HRPWM_INV_H
#define HRPWM_INV_H

#include <stdint.h>

typedef enum {
    INV_MODE_OFF = 0,
    INV_MODE_HALF,
    INV_MODE_FULL,
    INV_MODE_PHASE
} inv_mode_t;

typedef struct {
    uint32_t fsw_hz;
    uint32_t period_ticks;
    uint32_t deadtime_ticks;
    uint32_t phase_deg;     /* 180 = 反相全桥；<180 移相 */
    float duty;             /* 0.10～0.50，默认 0.50 */
    inv_mode_t mode;
    uint8_t enabled;
    uint8_t emb_latched;
} hrpwm_inv_t;

void hrpwm_inv_init(hrpwm_inv_t *h);
void hrpwm_inv_set_freq(hrpwm_inv_t *h, uint32_t fsw_hz);
void hrpwm_inv_set_phase(hrpwm_inv_t *h, uint32_t phase_deg);
void hrpwm_inv_set_duty(hrpwm_inv_t *h, float duty);
void hrpwm_inv_set_mode(hrpwm_inv_t *h, inv_mode_t mode);
void hrpwm_inv_enable(hrpwm_inv_t *h, uint8_t en);
void hrpwm_inv_emb_trip(hrpwm_inv_t *h);
void hrpwm_inv_emb_clear(hrpwm_inv_t *h);
uint32_t hrpwm_ns_to_ticks(uint32_t ns);

#endif
