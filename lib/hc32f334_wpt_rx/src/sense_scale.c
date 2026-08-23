#include "sense_scale.h"
#include "app_config.h"
#include "rx_math.h"

void sense_scale_init(void)
{
}

int32_t sense_vrect_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VRECT_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_vout_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VOUT_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_iout_lsb_to_ma(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * IOUT_FS_MA) / ADC_MAX_LSB);
}

uint16_t sense_mv_to_vout_lsb(int32_t mv)
{
    if (mv < 0) {
        mv = 0;
    }
    return (uint16_t)(((int64_t)mv * ADC_MAX_LSB) / VOUT_ADC_FS_MV);
}

uint16_t sense_ma_to_iout_lsb(int32_t ma)
{
    if (ma < 0) {
        ma = 0;
    }
    return (uint16_t)(((int64_t)ma * ADC_MAX_LSB) / IOUT_FS_MA);
}

void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng)
{
    eng->vrect_mv = sense_vrect_lsb_to_mv(raw->vrect);
    eng->vout_mv = sense_vout_lsb_to_mv(raw->vout);
    eng->iout_ma = sense_iout_lsb_to_ma(raw->iout);
    eng->prx_mw = rx_prx_mw(eng->vout_mv, eng->iout_ma);
    eng->ntc_coil_c = (int32_t)(((int64_t)raw->ntc_coil * 150) / ADC_MAX_LSB);
    eng->ntc_sr_c = (int32_t)(((int64_t)raw->ntc_sr * 150) / ADC_MAX_LSB);
}
