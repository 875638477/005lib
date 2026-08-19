#ifndef CSA_INA241_H
#define CSA_INA241_H

/*
 * High-side current sense: TI INA241A2 (20 V/V, -5 V to 110 V, PWM reject).
 * Shunt sits after each phase inductor (12 V rail) or on VIN high-side FET.
 */

#include "app_config.h"

#define CSA_PART_NAME           "INA241A2"
#define CSA_CM_MIN_V            (-5.0f)
#define CSA_CM_MAX_V            110.0f
#define CSA_OCP_PART_NAME       "INA293A2"
#define CSA_IOUT_PART_NAME      "ACS772-200B"

static inline float csa_iph_volts(float i_amp)
{
    return (i_amp * CSA_RSENSE_OHM * CSA_GAIN) + CSA_VREF_V;
}

static inline float csa_iph_amp(float v_adc)
{
    return (v_adc - CSA_VREF_V) / (CSA_GAIN * CSA_RSENSE_OHM);
}

static inline unsigned csa_ocp_dac_lsb(int i_ma)
{
    const float i_amp = (float)i_ma / 1000.0f;
    const float v = csa_iph_volts(i_amp);
    float lsb = v * (float)ADC_MAX_LSB / ADC_VREF_V;
    if (lsb < 0.0f) {
        lsb = 0.0f;
    }
    if (lsb > (float)ADC_MAX_LSB) {
        lsb = (float)ADC_MAX_LSB;
    }
    return (unsigned)(lsb + 0.5f);
}

#endif
