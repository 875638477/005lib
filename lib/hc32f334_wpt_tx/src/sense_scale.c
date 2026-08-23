#include "sense_scale.h"
#include "app_config.h"

void sense_scale_init(void)
{
}

int32_t sense_vin_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VIN_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_vbrg_lsb_to_mv(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * VBRG_ADC_FS_MV) / ADC_MAX_LSB);
}

int32_t sense_iin_lsb_to_ma(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * IIN_FS_MA) / ADC_MAX_LSB);
}

int32_t sense_icoil_lsb_to_ma(uint16_t lsb)
{
    return (int32_t)(((int64_t)lsb * ICOIL_FS_MA) / ADC_MAX_LSB);
}

uint16_t sense_mv_to_vbrg_lsb(int32_t mv)
{
    if (mv < 0) {
        mv = 0;
    }
    return (uint16_t)(((int64_t)mv * ADC_MAX_LSB) / VBRG_ADC_FS_MV);
}

uint16_t sense_ma_to_icoil_lsb(int32_t ma)
{
    if (ma < 0) {
        ma = 0;
    }
    return (uint16_t)(((int64_t)ma * ADC_MAX_LSB) / ICOIL_FS_MA);
}

void sense_raw_to_eng(const sense_raw_t *raw, sense_eng_t *eng)
{
    int64_t p;

    eng->vin_mv = sense_vin_lsb_to_mv(raw->vin);
    eng->vbrg_mv = sense_vbrg_lsb_to_mv(raw->vbrg);
    eng->iin_ma = sense_iin_lsb_to_ma(raw->iin);
    eng->icoil_ma = sense_icoil_lsb_to_ma(raw->icoil);
    p = (int64_t)eng->vin_mv * (int64_t)eng->iin_ma;
    eng->ptx_mw = (int32_t)(p / 1000);
    /* 10 kΩ / 100 kΩ NTC 简化：LSB 线性映到 0～150 °C，主机测试用 */
    eng->ntc_coil_c = (int32_t)(((int64_t)raw->ntc_coil * 150) / ADC_MAX_LSB);
    eng->ntc_mos_c = (int32_t)(((int64_t)raw->ntc_mos * 150) / ADC_MAX_LSB);
}
