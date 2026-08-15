/*
 * HPM6284IPA eLQFP144 pin map for 24V / 220V 4000W two-stage inverter.
 * ALT numbers follow HPM6200 series datasheet Rev 2.0 PINMUX.
 */

#ifndef HPM6284_INVERTER_PINMUX_H
#define HPM6284_INVERTER_PINMUX_H

#include <stdint.h>

/* IOC ALT select values used by this board */
#define INV_ALT_GPIO        0
#define INV_ALT_UART        2
#define INV_ALT_CAN         7
#define INV_ALT_PWM         16
#define INV_ALT_PWM2        17
#define INV_ALT_TRGM        16
#define INV_ALT_SDM         20

/* -------------------------------------------------------------------------- */
/* PWM0 : PSFB primary (HRPWM, 60 kHz)                                        */
/* -------------------------------------------------------------------------- */
#define PIN_PSFB_AH         IOC_PAD_PB12    /* PWM0_P_0, LQFP 119 */
#define PIN_PSFB_AL         IOC_PAD_PB13    /* PWM0_P_1, LQFP 118 */
#define PIN_PSFB_BH         IOC_PAD_PB14    /* PWM0_P_2, LQFP 117 */
#define PIN_PSFB_BL         IOC_PAD_PB15    /* PWM0_P_3, LQFP 116 */
#define PIN_PSFB_SR_H       IOC_PAD_PB16    /* PWM0_P_4 reserved */
#define PIN_PSFB_SR_L       IOC_PAD_PB17    /* PWM0_P_5 reserved */
#define PIN_PSFB_FAULT_OC   IOC_PAD_PB11    /* PWM0_FAULT_0, LQFP 121 */
#define PIN_PSFB_FAULT_AUX  IOC_PAD_PB10    /* PWM0_FAULT_1, LQFP 122 */

#define PWM_PSFB            HPM_PWM0
#define PWM_CH_PSFB_AH      0
#define PWM_CH_PSFB_AL      1
#define PWM_CH_PSFB_BH      2
#define PWM_CH_PSFB_BL      3

/* -------------------------------------------------------------------------- */
/* PWM1 : inverter H-bridge (HRPWM, 20 kHz unipolar SPWM)                     */
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
/* ADC analog pads (VIO_02 / Port C). Configure as analog, not ALT PWM.       */
/* -------------------------------------------------------------------------- */
#define PIN_ADC_I_PRI       IOC_PAD_PC04    /* ADC0_INA0, LQFP 82 */
#define PIN_ADC_I_AC        IOC_PAD_PC05    /* ADC0_INA1, LQFP 81 */
#define PIN_ADC_I_IN        IOC_PAD_PC06    /* ADC0_INA2, LQFP 80 */
#define PIN_ADC_I_BUS       IOC_PAD_PC07    /* ADC0_INA3, LQFP 79 */
#define PIN_ADC_V_IN        IOC_PAD_PC08    /* ADC1_INA0, LQFP 78 */
#define PIN_ADC_V_BUS       IOC_PAD_PC09    /* ADC1_INA1, LQFP 76 */
#define PIN_ADC_V_AC        IOC_PAD_PC10    /* ADC1_INA2, LQFP 75 */
#define PIN_ADC_V_N         IOC_PAD_PC11    /* ADC1_INA3, LQFP 74 */
#define PIN_ADC_NTC_MOS     IOC_PAD_PC12    /* ADC2_INA0, LQFP 73 */
#define PIN_ADC_NTC_XFMR    IOC_PAD_PC13    /* ADC2_INA1, LQFP 72 */
#define PIN_ADC_NTC_IGBT    IOC_PAD_PC14    /* ADC2_INA2, LQFP 71 */
#define PIN_ADC_P12V        IOC_PAD_PC16    /* ADC2_INA4, LQFP 69 */
#define PIN_ADC_P3V3        IOC_PAD_PC17    /* ADC2_INA5, LQFP 67 */

#define ADC_CH_I_PRI        0
#define ADC_CH_I_AC         1
#define ADC_CH_I_IN         2
#define ADC_CH_I_BUS        3
#define ADC_CH_V_IN         0   /* ADC1 */
#define ADC_CH_V_BUS        1
#define ADC_CH_V_AC         2
#define ADC_CH_V_N          3
#define ADC_CH_NTC_MOS      0   /* ADC2 */
#define ADC_CH_NTC_XFMR     1
#define ADC_CH_NTC_IGBT     2
#define ADC_CH_P12V         4
#define ADC_CH_P3V3         5

/* -------------------------------------------------------------------------- */
/* Optional SDM isolated bitstream (conflicts with fan PWM on PB24/25)        */
/* -------------------------------------------------------------------------- */
#define PIN_SDM_IAC_CLK     IOC_PAD_PB22
#define PIN_SDM_IAC_DAT     IOC_PAD_PB23
#define PIN_SDM_VAC_CLK     IOC_PAD_PB24
#define PIN_SDM_VAC_DAT     IOC_PAD_PB25

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
#define PIN_FAN1_FG         IOC_PAD_PB18
#define PIN_FAN2_FG         IOC_PAD_PB19

#ifdef __cplusplus
extern "C" {
#endif

void board_init_inverter_pins(void);

#ifdef __cplusplus
}
#endif

#endif /* HPM6284_INVERTER_PINMUX_H */
