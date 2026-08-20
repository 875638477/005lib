#ifndef SENSE_SCALE_H
#define SENSE_SCALE_H

#include <stdint.h>

typedef struct {
    int32_t vin_mv;
    int32_t vbus_mv;
    int32_t vmod_mv[4];
    int32_t iser_ma;
    int32_t iph_ma[4];
    int32_t vac_mv;
    int32_t iac_ma;
    int32_t temp_c;
} sense_eng_t;

typedef struct {
    uint16_t vin;
    uint16_t vbus;
    uint16_t vmod[4];
    uint16_t iser;
    uint16_t iph[4];
    uint16_t vac;
    uint16_t iac;
    uint16_t temp;
} sense_raw_t;

void sense_scale_init(void);
void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng);
int32_t sense_vin_lsb_to_mv(uint16_t lsb);
int32_t sense_vbus_lsb_to_mv(uint16_t lsb);
int32_t sense_vmod_lsb_to_mv(uint16_t lsb);
int32_t sense_iph_lsb_to_ma(uint16_t lsb);
int32_t sense_bipolar_ma(uint16_t lsb, int32_t fs_ma);
int32_t sense_vac_lsb_to_mv(uint16_t lsb);
uint16_t sense_mv_to_vbus_lsb(int32_t mv);
uint16_t sense_mv_to_vin_lsb(int32_t mv);

#endif
