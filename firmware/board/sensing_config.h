/*
 * Analog front-end scale factors for the 24V / 4kW HPM6284 inverter.
 * ADC assumed 16-bit, VREF = 3.3V. Recalibrate on the first board.
 */

#ifndef SENSING_CONFIG_H
#define SENSING_CONFIG_H

#define ADC_VREF_V              3.3f
#define ADC_FULL_SCALE          65535.0f
#define ADC_MID_COUNTS          32768.0f

/* V_IN : 100k + 10k divider, ratio 11 */
#define VIN_DIVIDER             11.0f
#define VIN_COUNTS_TO_V(c)      ((float)(c) * ADC_VREF_V / ADC_FULL_SCALE * VIN_DIVIDER)

/*
 * I_PRI : 0.2 mOhm shunt + INA240A1 (gain 20), mid-supply 1.65V
 * Vadc = 1.65 + 20 * I * 0.0002 = 1.65 + 0.004 * I
 */
#define IPRI_SHUNT_OHM          0.0002f
#define IPRI_AMP_GAIN           20.0f
#define IPRI_OFFSET_V           1.65f
#define IPRI_V_TO_A(v)          (((v) - IPRI_OFFSET_V) / (IPRI_AMP_GAIN * IPRI_SHUNT_OHM))
#define IPRI_COUNTS_TO_A(c)     IPRI_V_TO_A((float)(c) * ADC_VREF_V / ADC_FULL_SCALE)

/*
 * I_IN : ACS772-200B, 10 mV/A, 1.65V zero. Attenuate 3.65V@200A -> 3.3V
 * If no attenuator, clamp software range and pick 400A device.
 */
#define IIN_SENS_V_PER_A        0.010f
#define IIN_OFFSET_V            1.65f
#define IIN_ATTEN               (3.3f / 3.65f)
#define IIN_COUNTS_TO_A(c)      ((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IIN_OFFSET_V) \
                                 / (IIN_SENS_V_PER_A * IIN_ATTEN))

/*
 * I_AC : 0.5 mOhm shunt + AMC1302 (+/-50mV) + diff-to-single-end gain
 * Map +/-100A to 0.3V .. 3.0V around 1.65V.
 */
#define IAC_FS_A                100.0f
#define IAC_V_MIN               0.30f
#define IAC_V_MAX               3.00f
#define IAC_COUNTS_TO_A(c)      (((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IAC_V_MIN) \
                                 / (IAC_V_MAX - IAC_V_MIN) - 0.5f) * 2.0f * IAC_FS_A)

/* I_BUS : 2 mOhm + AMC1302, +/-25A -> +/-50mV */
#define IBUS_FS_A               25.0f
#define IBUS_COUNTS_TO_A(c)     (IAC_COUNTS_TO_A(c) * (IBUS_FS_A / IAC_FS_A))

/* V_BUS isolated divider, 0-500V -> 0-3.0V */
#define VBUS_FS_V               500.0f
#define VBUS_ADC_FS_V           3.0f
#define VBUS_COUNTS_TO_V(c)     ((float)(c) * ADC_VREF_V / ADC_FULL_SCALE * (VBUS_FS_V / VBUS_ADC_FS_V))

/* V_AC isolated, +/-400V -> 0.3-3.0V around 1.65V */
#define VAC_FS_V                400.0f
#define VAC_COUNTS_TO_V(c)      (((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IAC_V_MIN) \
                                 / (IAC_V_MAX - IAC_V_MIN) - 0.5f) * 2.0f * VAC_FS_V)

/* 10k NTC B3950 vs 10k, rough linearization done in software table */
#define NTC_R_PULLUP            10000.0f
#define NTC_R25                 10000.0f
#define NTC_BETA                3950.0f

#endif /* SENSING_CONFIG_H */
