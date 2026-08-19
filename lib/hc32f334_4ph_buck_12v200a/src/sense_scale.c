#include "sense_scale.h"
#include "app_config.h"
#include "csa_ina241.h"

void sense_scale_init(void)
{
}

int32_t sense_vout_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VOUT_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_vin_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VIN_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_iph_lsb_to_ma(uint16_t lsb)
{
    const float v = ((float)lsb * ADC_VREF_V) / (float)ADC_MAX_LSB;
    return (int32_t)(csa_iph_amp(v) * 1000.0f);
}

int32_t sense_iout_lsb_to_ma(uint16_t lsb)
{
#if IOUT_USE_HALL
    return (int32_t)(((int64_t)lsb * IOUT_HALL_FS_MA) / ADC_MAX_LSB);
#else
    return sense_iph_lsb_to_ma(lsb);
#endif
}

uint16_t sense_mv_to_vout_lsb(int32_t mv)
{
    if (mv < 0) {
        return 0;
    }
    int64_t lsb = ((int64_t)mv * ADC_MAX_LSB) / VOUT_ADC_FS_MV;
    if (lsb > ADC_MAX_LSB) {
        lsb = ADC_MAX_LSB;
    }
    return (uint16_t)lsb;
}

uint16_t sense_ma_to_iph_lsb(int32_t ma)
{
    const float v = csa_iph_volts((float)ma / 1000.0f);
    float lsb = v * (float)ADC_MAX_LSB / ADC_VREF_V;
    if (lsb < 0.0f) {
        lsb = 0.0f;
    }
    if (lsb > (float)ADC_MAX_LSB) {
        lsb = (float)ADC_MAX_LSB;
    }
    return (uint16_t)(lsb + 0.5f);
}

void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng)
{
    unsigned i;

    eng->vin_mv = sense_vin_lsb_to_mv(raw->vin);
    eng->vout_mv = sense_vout_lsb_to_mv(raw->vout);
    eng->iout_ma = sense_iout_lsb_to_ma(raw->iout);
    for (i = 0; i < PHASE_COUNT; ++i) {
        eng->iph_ma[i] = sense_iph_lsb_to_ma(raw->iph[i]);
    }
    /* 10 k NTC + 10 k, rough: 0.5 V ≈ 25 C scale placeholder */
    eng->temp_c = (int32_t)((raw->temp * 150) / ADC_MAX_LSB);
}
