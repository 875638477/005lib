/*
 * Protection thresholds for the 4-phase 24V / 4kW inverter.
 */

#ifndef PROTECTION_CONFIG_H
#define PROTECTION_CONFIG_H

#include <stdint.h>

#define VIN_UV_V                20.0f
#define VIN_UV_RECOVER_V        21.5f
#define VIN_OV_V                30.5f

#define IIN_OC_SLOW_A           240.0f
#define IIN_OC_SLOW_MS          100
#define IIN_OC_HW_A             350.0f

#define VBUS_UV_V               320.0f
#define VBUS_NOM_V              380.0f
#define VBUS_OV_SW_V            420.0f
#define VBUS_OV_HW_V            430.0f

#define IPH_LIM_A               70.0f
#define IPH_HW_A                80.0f
#define IPH_TRIP_COUNT          50
#define IPH_UNBAL_RATIO         0.25f
#define IPH_UNBAL_MS            200
#define IPRI_SUM_LIM_A          260.0f

#define IAC_RMS_OL_A            28.0f
#define IAC_RMS_OL_S            10
#define IAC_SURGE_A             45.0f
#define IAC_SURGE_S             1
#define IAC_HW_A                70.0f

#define VAC_RMS_OV_V            242.0f
#define VAC_PEAK_OV_V           380.0f

#define TEMP_FAN_ON_C           45.0f
#define TEMP_DERATE_C           85.0f
#define TEMP_SHUT_C             105.0f
#define TEMP_RECOVER_HYST_C     15.0f

#define DCDC_F_SW_HZ            60000
#define DCDC_PHASES             4
#define DCDC_INTERLEAVE_DEG     90
#define INV_F_SW_HZ             20000
#define INV_F_OUT_HZ            50
#define INV_DEADTIME_NS         500
#define DCDC_DEADTIME_NS        200
#define DCDC_MAX_DUTY           0.85f

#define ACMP_DAC_FROM_V(v)      ((uint8_t)((v) * 255.0f / 3.3f + 0.5f))

enum inv_fault_bit {
    FAULT_VIN_UV        = 1u << 0,
    FAULT_VIN_OV        = 1u << 1,
    FAULT_IIN_OC        = 1u << 2,
    FAULT_VBUS_UV       = 1u << 3,
    FAULT_VBUS_OV       = 1u << 4,
    FAULT_IPH1_OC       = 1u << 5,
    FAULT_IPH2_OC       = 1u << 6,
    FAULT_IPH3_OC       = 1u << 7,
    FAULT_IPH4_OC       = 1u << 8,
    FAULT_IPH_UNBAL     = 1u << 9,
    FAULT_IAC_OC        = 1u << 10,
    FAULT_VAC_OV        = 1u << 11,
    FAULT_SHORT         = 1u << 12,
    FAULT_OT_MOS        = 1u << 13,
    FAULT_OT_XFMR       = 1u << 14,
    FAULT_OT_IGBT       = 1u << 15,
    FAULT_FAN           = 1u << 16,
    FAULT_ESTOP         = 1u << 17,
    FAULT_HW_PWM0       = 1u << 18,
    FAULT_HW_PWM1       = 1u << 19,
    FAULT_HW_PWM3       = 1u << 20,
    FAULT_WATCHDOG      = 1u << 21,
    FAULT_AUX_RAIL      = 1u << 22
};

enum inv_state {
    INV_STATE_INIT = 0,
    INV_STATE_SELFTEST,
    INV_STATE_PRECHARGE,
    INV_STATE_DCDC_SOFTSTART,
    INV_STATE_INV_SOFTSTART,
    INV_STATE_RUN,
    INV_STATE_FAULT,
    INV_STATE_SHUTDOWN
};

#endif /* PROTECTION_CONFIG_H */
