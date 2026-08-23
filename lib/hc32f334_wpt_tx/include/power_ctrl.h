#ifndef POWER_CTRL_H
#define POWER_CTRL_H

#include <stdint.h>
#include "hrpwm_inv.h"

typedef struct {
    int32_t vbrg_mv;
    int32_t vbrg_min_mv;
    int32_t vbrg_max_mv;
    uint32_t fsw_hz;
    int16_t cep;            /* RX Control Error，有符号 */
    uint8_t vbrg_sat;
} power_ctrl_t;

void power_ctrl_init(power_ctrl_t *c);
void power_ctrl_apply_cep(power_ctrl_t *c, int16_t cep);
void power_ctrl_clamp_for_vin(power_ctrl_t *c, int32_t vin_mv);
void power_ctrl_to_inv(const power_ctrl_t *c, hrpwm_inv_t *inv);

#endif
