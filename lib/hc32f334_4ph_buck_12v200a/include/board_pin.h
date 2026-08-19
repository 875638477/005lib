#ifndef BOARD_PIN_H
#define BOARD_PIN_H

/*
 * HC32F334KATI-LQFP64 pin map
 * Source: DS_HC32F334 Rev1.10 Table 2-1 (HRPWM / Analog columns).
 */

typedef enum {
    PORT_A = 0,
    PORT_B,
    PORT_C,
    PORT_D,
    PORT_F
} board_port_t;

typedef struct {
    board_port_t port;
    unsigned pin;
    const char *func;
    unsigned lqfp64;
} board_pwm_pin_t;

#define HRPWM_UNIT_PH1      1U
#define HRPWM_UNIT_PH2      2U
#define HRPWM_UNIT_PH3      3U
#define HRPWM_UNIT_PH4      4U

#define PIN_PH1_HS_PORT     PORT_C
#define PIN_PH1_HS          6U          /* PC6  HRPWM_1_PWMA  pin 37 */
#define PIN_PH1_LS_PORT     PORT_C
#define PIN_PH1_LS          7U          /* PC7  HRPWM_1_PWMB  pin 38 */

#define PIN_PH2_HS_PORT     PORT_A
#define PIN_PH2_HS          8U          /* PA8  HRPWM_2_PWMA  pin 41 */
#define PIN_PH2_LS_PORT     PORT_A
#define PIN_PH2_LS          9U          /* PA9  HRPWM_2_PWMB  pin 42 */

#define PIN_PH3_HS_PORT     PORT_A
#define PIN_PH3_HS          10U         /* PA10 HRPWM_3_PWMA  pin 43 */
#define PIN_PH3_LS_PORT     PORT_A
#define PIN_PH3_LS          11U         /* PA11 HRPWM_3_PWMB  pin 44 */

#define PIN_PH4_HS_PORT     PORT_B
#define PIN_PH4_HS          12U         /* PB12 HRPWM_4_PWMA  pin 33 */
#define PIN_PH4_LS_PORT     PORT_B
#define PIN_PH4_LS          13U         /* PB13 HRPWM_4_PWMB  pin 34 */

#define PIN_IPH1_PORT       PORT_A
#define PIN_IPH1            0U          /* PA0 ADC1_IN0 */
#define PIN_IPH2_PORT       PORT_A
#define PIN_IPH2            1U          /* PA1 ADC1_IN1 */
#define PIN_IPH3_PORT       PORT_A
#define PIN_IPH3            2U          /* PA2 ADC1_IN2 */
#define PIN_IPH4_PORT       PORT_A
#define PIN_IPH4            3U          /* PA3 ADC1_IN3 */

#define PIN_VOUT_PORT       PORT_A
#define PIN_VOUT            6U          /* PA6 ADC123_IN6 */
#define PIN_VIN_PORT        PORT_C
#define PIN_VIN             0U          /* PC0 ADC12_IN10 */
#define PIN_IOUT_PORT       PORT_B
#define PIN_IOUT            0U          /* PB0 ADC123_IN8 */
#define PIN_TEMP_PORT       PORT_C
#define PIN_TEMP            1U          /* PC1 ADC12_IN11 */

#define PIN_DAC_OCP_PORT    PORT_A
#define PIN_DAC_OCP         4U          /* PA4 DAC1_OUT1 + CMPx_INM1 */
#define PIN_CMP1_OCP_PORT   PORT_A
#define PIN_CMP1_OCP        7U          /* PA7 CMP1_INP1 */
#define PIN_CMP2_OVP_PORT   PORT_A
#define PIN_CMP2_OVP        3U          /* PA3 CMP2_INP2; prefer dedicated divider */
#define PIN_EMB_FAULT_PORT  PORT_C
#define PIN_EMB_FAULT       5U          /* PC5 EMB_PORT1_IN3 */

#define PIN_ENABLE_PORT     PORT_D
#define PIN_ENABLE          2U
#define PIN_LED_RUN_PORT    PORT_C
#define PIN_LED_RUN         2U
#define PIN_LED_FAULT_PORT  PORT_C
#define PIN_LED_FAULT       3U

#define ADC_CH_IPH1         0U
#define ADC_CH_IPH2         1U
#define ADC_CH_IPH3         2U
#define ADC_CH_IPH4         3U
#define ADC_CH_VOUT         6U
#define ADC_CH_VIN          10U
#define ADC_CH_IOUT         8U
#define ADC_CH_TEMP         11U

#define ADC_UNIT_IPH_A      1U          /* ADC1: IPH1, IPH2 */
#define ADC_UNIT_IPH_B      2U          /* ADC2: IPH3, IPH4 */
#define ADC_UNIT_BUS        3U          /* ADC3: VIN, VOUT, IOUT */

static const board_pwm_pin_t k_hrpwm_pins[8] = {
    {PORT_C, 6U,  "HRPWM_1_PWMA", 37U},
    {PORT_C, 7U,  "HRPWM_1_PWMB", 38U},
    {PORT_A, 8U,  "HRPWM_2_PWMA", 41U},
    {PORT_A, 9U,  "HRPWM_2_PWMB", 42U},
    {PORT_A, 10U, "HRPWM_3_PWMA", 43U},
    {PORT_A, 11U, "HRPWM_3_PWMB", 44U},
    {PORT_B, 12U, "HRPWM_4_PWMA", 33U},
    {PORT_B, 13U, "HRPWM_4_PWMB", 34U},
};

#endif
