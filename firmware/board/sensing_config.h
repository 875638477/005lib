/*
 * Analog front-end scale factors for the 4-phase 24V / 4kW inverter.
 * ADC assumed 16-bit, VREF = 3.3V. Recalibrate per phase on the first board.
 */

#ifndef SENSING_CONFIG_H
#define SENSING_CONFIG_H

#define ADC_VREF_V              3.3f
#define ADC_FULL_SCALE          65535.0f
#define ADC_MID_COUNTS          32768.0f

#define VIN_DIVIDER             11.0f
#define VIN_COUNTS_TO_V(c)      ((float)(c) * ADC_VREF_V / ADC_FULL_SCALE * VIN_DIVIDER)

/*
 * Per-phase primary current: 1.0 mOhm shunt + INA240A1 (gain 20), mid 1.65V
 * Vadc = 1.65 + 20 * I * 0.001 = 1.65 + 0.02 * I
 * 46A -> 2.57V, 80A HW trip -> 3.25V
 */
#define IPH_SHUNT_OHM           0.001f
#define IPH_AMP_GAIN            20.0f
#define IPH_OFFSET_V            1.65f
#define IPH_V_TO_A(v)           (((v) - IPH_OFFSET_V) / (IPH_AMP_GAIN * IPH_SHUNT_OHM))
#define IPH_COUNTS_TO_A(c)      IPH_V_TO_A((float)(c) * ADC_VREF_V / ADC_FULL_SCALE)

#define IIN_SENS_V_PER_A        0.010f
#define IIN_OFFSET_V            1.65f
#define IIN_ATTEN               (3.3f / 3.65f)
#define IIN_COUNTS_TO_A(c)      ((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IIN_OFFSET_V) \
                                 / (IIN_SENS_V_PER_A * IIN_ATTEN))

#define IAC_FS_A                100.0f
#define IAC_V_MIN               0.30f
#define IAC_V_MAX               3.00f
#define IAC_COUNTS_TO_A(c)      (((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IAC_V_MIN) \
                                 / (IAC_V_MAX - IAC_V_MIN) - 0.5f) * 2.0f * IAC_FS_A)

#define VBUS_FS_V               500.0f
#define VBUS_ADC_FS_V           3.0f
#define VBUS_COUNTS_TO_V(c)     ((float)(c) * ADC_VREF_V / ADC_FULL_SCALE * (VBUS_FS_V / VBUS_ADC_FS_V))

#define VAC_FS_V                400.0f
#define VAC_COUNTS_TO_V(c)      (((((float)(c) * ADC_VREF_V / ADC_FULL_SCALE) - IAC_V_MIN) \
                                 / (IAC_V_MAX - IAC_V_MIN) - 0.5f) * 2.0f * VAC_FS_V)

#define NTC_R_PULLUP            10000.0f
#define NTC_R25                 10000.0f
#define NTC_BETA                3950.0f

#endif /* SENSING_CONFIG_H */
