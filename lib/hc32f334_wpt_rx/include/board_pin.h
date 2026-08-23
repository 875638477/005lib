#ifndef BOARD_PIN_H
#define BOARD_PIN_H

/*
 * HC32F334KATI-LQFP64 pin map for single-coil WPT RX.
 * Source: DS_HC32F334 Rev1.10 Table 2-1.
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

#define PIN_SRA_HS_PORT     PORT_C
#define PIN_SRA_HS          6U          /* PC6  HRPWM_1_PWMA  pin 37 */
#define PIN_SRA_LS_PORT     PORT_C
#define PIN_SRA_LS          7U          /* PC7  HRPWM_1_PWMB  pin 38 */

#define PIN_SRB_HS_PORT     PORT_A
#define PIN_SRB_HS          8U          /* PA8  HRPWM_2_PWMA  pin 41 */
#define PIN_SRB_LS_PORT     PORT_A
#define PIN_SRB_LS          9U          /* PA9  HRPWM_2_PWMB  pin 42 */

#define PIN_BUCK_HS_PORT    PORT_A
#define PIN_BUCK_HS         10U         /* PA10 HRPWM_3_PWMA  pin 43 */
#define PIN_BUCK_LS_PORT    PORT_A
#define PIN_BUCK_LS         11U         /* PA11 HRPWM_3_PWMB  pin 44 */

#define PIN_ASK_PORT        PORT_C
#define PIN_ASK             4U          /* PC4 GPIO ASK 调制 */

#define PIN_VRECT_PORT      PORT_A
#define PIN_VRECT           0U          /* PA0 ADC1_IN0 */
#define PIN_VOUT_PORT       PORT_A
#define PIN_VOUT            1U          /* PA1 ADC1_IN1 */
#define PIN_IOUT_PORT       PORT_A
#define PIN_IOUT            2U          /* PA2 ADC1_IN2 */
#define PIN_VCOIL_PORT      PORT_A
#define PIN_VCOIL           6U          /* PA6 ADC123_IN6 */
#define PIN_NTC_COIL_PORT   PORT_C
#define PIN_NTC_COIL        1U          /* PC1 ADC12_IN11 */
#define PIN_NTC_SR_PORT     PORT_B
#define PIN_NTC_SR          0U          /* PB0 ADC123_IN8 */

#define PIN_DAC_OVP_PORT    PORT_A
#define PIN_DAC_OVP         4U
#define PIN_CMP1_SR_PORT    PORT_A
#define PIN_CMP1_SR         7U          /* PA7 CMP1 线圈极性 */
#define PIN_EMB_FAULT_PORT  PORT_C
#define PIN_EMB_FAULT       5U

#define PIN_LED_RUN_PORT    PORT_C
#define PIN_LED_RUN         2U
#define PIN_LED_FAULT_PORT  PORT_C
#define PIN_LED_FAULT       3U

static const board_pwm_pin_t k_hrpwm_pins[6] = {
    {PORT_C, 6U,  "HRPWM_1_PWMA", 37U},
    {PORT_C, 7U,  "HRPWM_1_PWMB", 38U},
    {PORT_A, 8U,  "HRPWM_2_PWMA", 41U},
    {PORT_A, 9U,  "HRPWM_2_PWMB", 42U},
    {PORT_A, 10U, "HRPWM_3_PWMA", 43U},
    {PORT_A, 11U, "HRPWM_3_PWMB", 44U},
};

#endif
