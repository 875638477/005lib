#ifndef HRPWM_SR_H
#define HRPWM_SR_H

#include <stdint.h>

typedef enum {
    SR_MODE_DIODE = 0,  /* PWM 全关，走体二极管/肖特基 */
    SR_MODE_SYNC
} sr_mode_t;

typedef struct {
    sr_mode_t mode;
    uint8_t polarity;   /* 1 = CMP 反相 */
    uint8_t enabled;
    uint8_t emb_latched;
    uint32_t deadtime_ticks;
} hrpwm_sr_t;

void hrpwm_sr_init(hrpwm_sr_t *h);
void hrpwm_sr_set_mode(hrpwm_sr_t *h, sr_mode_t mode);
void hrpwm_sr_enable(hrpwm_sr_t *h, uint8_t en);
void hrpwm_sr_emb_trip(hrpwm_sr_t *h);
void hrpwm_sr_emb_clear(hrpwm_sr_t *h);
uint32_t hrpwm_ns_to_ticks(uint32_t ns);

#endif
