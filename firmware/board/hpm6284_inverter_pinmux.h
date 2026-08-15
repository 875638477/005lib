/*
 * HPM6284IPA eLQFP144 pin map
 * Front-end: 4-phase interleaved full-bridge (PWM0 + PWM3)
 * Inverter : unipolar SPWM H-bridge (PWM1)
 * ALT numbers follow HPM6200 series datasheet Rev 2.0 PINMUX.
 */

#ifndef HPM6284_INVERTER_PINMUX_H
#define HPM6284_INVERTER_PINMUX_H

#include <stdint.h>

#define INV_ALT_GPIO        0
#define INV_ALT_UART        2
#define INV_ALT_CAN         7
#define INV_ALT_PWM         16  /* PWM0 / PWM1 */
#define INV_ALT_PWM23       17  /* PWM2 / PWM3 */
#define INV_ALT_SDM         20

#define DCDC_PHASE_COUNT    4

/* -------------------------------------------------------------------------- */
/* PWM0 HRPWM : phase 1 (0 deg) + phase 2 (90 deg)                            */
/* -------------------------------------------------------------------------- */
#define PIN_PH1_AH          IOC_PAD_PB12    /* PWM0_P_0, LQFP 119 */
#define PIN_PH1_AL          IOC_PAD_PB13    /* PWM0_P_1, LQFP 118 */
#define PIN_PH1_BH          IOC_PAD_PB14    /* PWM0_P_2, LQFP 117 */
#define PIN_PH1_BL          IOC_PAD_PB15    /* PWM0_P_3, LQFP 116 */
#define PIN_PH2_AH          IOC_PAD_PB16    /* PWM0_P_4, LQFP 115 */
#define PIN_PH2_AL          IOC_PAD_PB17    /* PWM0_P_5, LQFP 114 */
#define PIN_PH2_BH          IOC_PAD_PB18    /* PWM0_P_6, LQFP 112 */
#define PIN_PH2_BL          IOC_PAD_PB19    /* PWM0_P_7, LQFP 111 */
#define PIN_DCDC_FAULT_OC   IOC_PAD_PB11    /* PWM0_FAULT_0, LQFP 121 */
#define PIN_DCDC_FAULT_AUX  IOC_PAD_PB10    /* PWM0_FAULT_1, LQFP 122 */

#define PWM_PH12            HPM_PWM0
#define PWM_CH_PH1_AH       0
#define PWM_CH_PH1_AL       1
#define PWM_CH_PH1_BH       2
#define PWM_CH_PH1_BL       3
#define PWM_CH_PH2_AH       4
#define PWM_CH_PH2_AL       5
#define PWM_CH_PH2_BH       6
#define PWM_CH_PH2_BL       7

/* -------------------------------------------------------------------------- */
/* PWM3 : phase 3 (180 deg) + phase 4 (270 deg)                               */
/* -------------------------------------------------------------------------- */
#define PIN_PH3_AH          IOC_PAD_PA20    /* PWM3_P_00, LQFP 3 */
#define PIN_PH3_AL          IOC_PAD_PA21    /* PWM3_P_01, LQFP 2 */
#define PIN_PH3_BH          IOC_PAD_PA22    /* PWM3_P_02, LQFP 1 */
#define PIN_PH3_BL          IOC_PAD_PA23    /* PWM3_P_03, LQFP 144 */
#define PIN_PH4_AH          IOC_PAD_PA24    /* PWM3_P_04, LQFP 143 */
#define PIN_PH4_AL          IOC_PAD_PA25    /* PWM3_P_05, LQFP 142 */
#define PIN_PH4_BH          IOC_PAD_PA26    /* PWM3_P_06, LQFP 141 */
#define PIN_PH4_BL          IOC_PAD_PA27    /* PWM3_P_07, LQFP 139 */

#define PWM_PH34            HPM_PWM3
#define PWM_CH_PH3_AH       0
#define PWM_CH_PH3_AL       1
#define PWM_CH_PH3_BH       2
#define PWM_CH_PH3_BL       3
#define PWM_CH_PH4_AH       4
#define PWM_CH_PH4_AL       5
#define PWM_CH_PH4_BH       6
#define PWM_CH_PH4_BL       7

/* -------------------------------------------------------------------------- */
/* PWM1 HRPWM : inverter H-bridge                                             */
/* -------------------------------------------------------------------------- */
#define PIN_INV_AH          IOC_PAD_PB00    /* PWM1_P_0, LQFP 134 */
#define PIN_INV_AL          IOC_PAD_PB01    /* PWM1_P_1, LQFP 132 */
#define PIN_INV_BH          IOC_PAD_PB02    /* PWM1_P_2, LQFP 131 */
#define PIN_INV_BL          IOC_PAD_PB03    /* PWM1_P_3, LQFP 130 */
#define PIN_INV_FAULT_OC    IOC_PAD_PB08    /* PWM1_FAULT_0, LQFP 124 */
#define PIN_INV_FAULT_AUX   IOC_PAD_PB09    /* PWM1_FAULT_1, LQFP 123 */

#define PWM_INV             HPM_PWM1
#define PWM_CH_INV_AH       0
#define PWM_CH_INV_AL       1
#define PWM_CH_INV_BH       2
#define PWM_CH_INV_BL       3

/* -------------------------------------------------------------------------- */
/* PWM2 : cooling                                                             */
/* -------------------------------------------------------------------------- */
#define PIN_FAN1            IOC_PAD_PB24    /* PWM2_P_00, LQFP 106 */
#define PIN_FAN2            IOC_PAD_PB25    /* PWM2_P_01, LQFP 105 */

/* -------------------------------------------------------------------------- */
/* ADC analog pads (VIO_02 / Port C)                                          */
/* -------------------------------------------------------------------------- */
#define PIN_ADC_I_PH1       IOC_PAD_PC04    /* ADC0_INA0, LQFP 82 */
#define PIN_ADC_I_PH2       IOC_PAD_PC05    /* ADC0_INA1, LQFP 81 */
#define PIN_ADC_I_PH3       IOC_PAD_PC06    /* ADC0_INA2, LQFP 80 */
#define PIN_ADC_I_PH4       IOC_PAD_PC07    /* ADC0_INA3, LQFP 79 */
#define PIN_ADC_I_AC        IOC_PAD_PC12    /* ADC0_INA8, LQFP 73 */
#define PIN_ADC_V_IN        IOC_PAD_PC08    /* ADC1_INA0, LQFP 78 */
#define PIN_ADC_V_BUS       IOC_PAD_PC09    /* ADC1_INA1, LQFP 76 */
#define PIN_ADC_V_AC        IOC_PAD_PC10    /* ADC1_INA2, LQFP 75 */
#define PIN_ADC_I_IN        IOC_PAD_PC11    /* ADC1_INA3, LQFP 74 */
#define PIN_ADC_NTC_MOS     IOC_PAD_PC13    /* ADC2_INA1, LQFP 72 */
#define PIN_ADC_NTC_XFMR    IOC_PAD_PC14    /* ADC2_INA2, LQFP 71 */
#define PIN_ADC_NTC_IGBT    IOC_PAD_PC16    /* ADC2_INA4, LQFP 69 */
#define PIN_ADC_P12V        IOC_PAD_PC17    /* ADC2_INA5, LQFP 67 */

#define ADC_CH_I_PH1        0
#define ADC_CH_I_PH2        1
#define ADC_CH_I_PH3        2
#define ADC_CH_I_PH4        3
#define ADC_CH_I_AC         8   /* ADC0 */
#define ADC_CH_V_IN         0   /* ADC1 */
#define ADC_CH_V_BUS        1
#define ADC_CH_V_AC         2
#define ADC_CH_I_IN         3
#define ADC_CH_NTC_MOS      1   /* ADC2 */
#define ADC_CH_NTC_XFMR     2
#define ADC_CH_NTC_IGBT     4
#define ADC_CH_P12V         5

/* -------------------------------------------------------------------------- */
/* GPIO / comms                                                               */
/* -------------------------------------------------------------------------- */
#define PIN_UART0_TX        IOC_PAD_PA30
#define PIN_UART0_RX        IOC_PAD_PA31
#define PIN_CAN0_TX         IOC_PAD_PA29

#define PIN_RLY_INPUT       IOC_PAD_PB20
#define PIN_RLY_PRECHARGE   IOC_PAD_PB21
#define PIN_RLY_OUTPUT      IOC_PAD_PB30
#define PIN_BUZZER          IOC_PAD_PB31
#define PIN_LED_RUN         IOC_PAD_PC00
#define PIN_LED_FAULT       IOC_PAD_PC01
#define PIN_ESTOP           IOC_PAD_PC02
#define PIN_FAN1_FG         IOC_PAD_PA28
#define PIN_FAN2_FG         IOC_PAD_PY01

#ifdef __cplusplus
extern "C" {
#endif

void board_init_inverter_pins(void);

#ifdef __cplusplus
}
#endif

#endif /* HPM6284_INVERTER_PINMUX_H */
