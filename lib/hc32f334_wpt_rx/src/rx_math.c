#include "rx_math.h"
#include "app_config.h"

#include <math.h>

uint32_t rx_fres_hz(uint32_t lrx_nh, uint32_t cs_nf)
{
    double l = (double)lrx_nh * 1e-9;
    double c = (double)cs_nf * 1e-9;

    if (l <= 0.0 || c <= 0.0) {
        return 0U;
    }
    return (uint32_t)(1.0 / (2.0 * 3.141592653589793 * sqrt(l * c)) + 0.5);
}

int16_t rx_cep_from_vrect(int32_t vrect_mv, int32_t target_mv)
{
    int32_t e = (target_mv - vrect_mv) / CEP_MV_PER_LSB;

    if (e > 127) {
        e = 127;
    }
    if (e < -128) {
        e = -128;
    }
    return (int16_t)e;
}

int32_t rx_prx_mw(int32_t vout_mv, int32_t iout_ma)
{
    return (int32_t)(((int64_t)vout_mv * (int64_t)iout_ma) / 1000);
}

uint8_t rx_rpp8_from_mw(int32_t prx_mw)
{
    int32_t v = prx_mw / 500;

    if (v < 0) {
        v = 0;
    }
    if (v > 255) {
        v = 255;
    }
    return (uint8_t)v;
}

uint16_t rx_rpp16_from_mw(int32_t prx_mw)
{
    if (prx_mw < 0) {
        return 0U;
    }
    if (prx_mw > 65535) {
        return 65535U;
    }
    return (uint16_t)prx_mw;
}
