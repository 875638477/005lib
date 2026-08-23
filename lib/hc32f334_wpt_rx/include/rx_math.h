#ifndef RX_MATH_H
#define RX_MATH_H

#include <stdint.h>

uint32_t rx_fres_hz(uint32_t lrx_nh, uint32_t cs_nf);
int16_t rx_cep_from_vrect(int32_t vrect_mv, int32_t target_mv);
uint8_t rx_rpp8_from_mw(int32_t prx_mw);
int32_t rx_prx_mw(int32_t vout_mv, int32_t iout_ma);
uint16_t rx_rpp16_from_mw(int32_t prx_mw);

#endif
