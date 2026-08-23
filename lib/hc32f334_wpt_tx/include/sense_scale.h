#ifndef SENSE_SCALE_H
#define SENSE_SCALE_H

#include <stdint.h>

typedef struct {
    int32_t vin_mv;
    int32_t vbrg_mv;
    int32_t iin_ma;
    int32_t icoil_ma;
    int32_t ptx_mw;
    int32_t ntc_coil_c;
    int32_t ntc_mos_c;
} sense_eng_t;

typedef struct {
    uint16_t vin;
    uint16_t vbrg;
    uint16_t iin;
    uint16_t icoil;
    uint16_t ntc_coil;
    uint16_t ntc_mos;
} sense_raw_t;

void sense_scale_init(void);
void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng);
int32_t sense_vin_lsb_to_mv(uint16_t lsb);
int32_t sense_vbrg_lsb_to_mv(uint16_t lsb);
int32_t sense_iin_lsb_to_ma(uint16_t lsb);
int32_t sense_icoil_lsb_to_ma(uint16_t lsb);
uint16_t sense_mv_to_vbrg_lsb(int32_t mv);
uint16_t sense_ma_to_icoil_lsb(int32_t ma);

#endif
