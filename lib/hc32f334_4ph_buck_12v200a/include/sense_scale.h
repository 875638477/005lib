#ifndef SENSE_SCALE_H
#define SENSE_SCALE_H

#include <stdint.h>

typedef struct {
    int32_t vin_mv;
    int32_t vout_mv;
    int32_t iout_ma;
    int32_t iph_ma[4];
    int32_t temp_c;
} sense_eng_t;

typedef struct {
    uint16_t vin;
    uint16_t vout;
    uint16_t iout;
    uint16_t iph[4];
    uint16_t temp;
} sense_raw_t;

void sense_scale_init(void);
void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng);
int32_t sense_vout_lsb_to_mv(uint16_t lsb);
int32_t sense_vin_lsb_to_mv(uint16_t lsb);
int32_t sense_iph_lsb_to_ma(uint16_t lsb);
int32_t sense_iout_lsb_to_ma(uint16_t lsb);
uint16_t sense_mv_to_vout_lsb(int32_t mv);
uint16_t sense_ma_to_iph_lsb(int32_t ma);

#endif
