#include "sense_scale.h"
#include "app_config.h"

void sense_scale_init(void)
{
}

int32_t sense_vin_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VIN_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_vbus_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VBUS_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_vmod_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VMOD_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_iph_lsb_to_ma(uint16_t lsb)
{
    const float v = ((float)lsb * ADC_VREF_V) / (float)ADC_MAX_LSB;
    const float i = (v - CSA_VREF_V) / (CSA_GAIN * CSA_RSENSE_OHM);
    return (int32_t)(i * 1000.0f);
}

int32_t sense_bipolar_ma(uint16_t lsb, int32_t fs_ma)
{
    const int32_t mid = ADC_MAX_LSB / 2;
    return (int32_t)(((int64_t)((int32_t)lsb - mid) * fs_ma) / mid);
}

int32_t sense_vac_lsb_to_mv(uint16_t lsb)
{
    const int32_t mid = ADC_MAX_LSB / 2;
    return (int32_t)(((int64_t)((int32_t)lsb - mid) * VAC_FS_PK_MV) / mid);
}

uint16_t sense_mv_to_vbus_lsb(int32_t mv)
{
    int64_t lsb;

    if (mv < 0) {
        return 0;
    }
    lsb = ((int64_t)mv * ADC_MAX_LSB) / VBUS_ADC_FS_MV;
    if (lsb > ADC_MAX_LSB) {
        lsb = ADC_MAX_LSB;
    }
    return (uint16_t)lsb;
}

uint16_t sense_mv_to_vin_lsb(int32_t mv)
{
    int64_t lsb;

    if (mv < 0) {
        return 0;
    }
    lsb = ((int64_t)mv * ADC_MAX_LSB) / VIN_ADC_FS_MV;
    if (lsb > ADC_MAX_LSB) {
        lsb = ADC_MAX_LSB;
    }
    return (uint16_t)lsb;
}

void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng)
{
    unsigned i;

    eng->vin_mv = sense_vin_lsb_to_mv(raw->vin);
    eng->vbus_mv = sense_vbus_lsb_to_mv(raw->vbus);
    eng->iser_ma = sense_bipolar_ma(raw->iser, ISER_FS_MA);
    eng->vac_mv = sense_vac_lsb_to_mv(raw->vac);
    eng->iac_ma = sense_bipolar_ma(raw->iac, IAC_FS_MA);
    for (i = 0; i < PHASE_COUNT; ++i) {
        eng->vmod_mv[i] = sense_vmod_lsb_to_mv(raw->vmod[i]);
        eng->iph_ma[i] = sense_iph_lsb_to_ma(raw->iph[i]);
    }
    eng->temp_c = (int32_t)((raw->temp * 150) / ADC_MAX_LSB);
}
