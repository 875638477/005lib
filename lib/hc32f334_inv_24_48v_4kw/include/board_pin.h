#ifndef BOARD_PIN_H
#define BOARD_PIN_H

/*
 * HC32F334KATI-LQFP64 pin map
 * HRPWM1-4: DS_HC32F334 Rev1.10 Table 2-1 (same as 12 V / 200 A board).
 * HRPWM5/6 and Timer6: verify alternate function in the same table before tape-out.
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

#define HRPWM_UNIT_FB1_A    1U
#define HRPWM_UNIT_FB2_A    2U
#define HRPWM_UNIT_FB3_A    3U
#define HRPWM_UNIT_FB4_A    4U
#define HRPWM_UNIT_INV_A    5U
#define HRPWM_UNIT_INV_B    6U

#define TMR6_UNIT_FB1_B     1U
#define TMR6_UNIT_FB2_B     2U
#define TMR6_UNIT_FB3_B     3U
#define TMR6_UNIT_FB4_B     4U

#define PIN_FB1_AH_PORT     PORT_C
#define PIN_FB1_AH          6U          /* PC6  HRPWM_1_PWMA  pin 37 */
#define PIN_FB1_AL_PORT     PORT_C
#define PIN_FB1_AL          7U          /* PC7  HRPWM_1_PWMB  pin 38 */
#define PIN_FB2_AH_PORT     PORT_A
#define PIN_FB2_AH          8U          /* PA8  HRPWM_2_PWMA  pin 41 */
#define PIN_FB2_AL_PORT     PORT_A
#define PIN_FB2_AL          9U          /* PA9  HRPWM_2_PWMB  pin 42 */
#define PIN_FB3_AH_PORT     PORT_A
#define PIN_FB3_AH          10U         /* PA10 HRPWM_3_PWMA  pin 43 */
#define PIN_FB3_AL_PORT     PORT_A
#define PIN_FB3_AL          11U         /* PA11 HRPWM_3_PWMB  pin 44 */
#define PIN_FB4_AH_PORT     PORT_B
#define PIN_FB4_AH          12U         /* PB12 HRPWM_4_PWMA  pin 33 */
#define PIN_FB4_AL_PORT     PORT_B
#define PIN_FB4_AL          13U         /* PB13 HRPWM_4_PWMB  pin 34 */

#define PIN_FB1_BH_PORT     PORT_B
#define PIN_FB1_BH          4U          /* PB4  TIM6_1_PWMA */
#define PIN_FB1_BL_PORT     PORT_B
#define PIN_FB1_BL          5U
#define PIN_FB2_BH_PORT     PORT_B
#define PIN_FB2_BH          6U
#define PIN_FB2_BL_PORT     PORT_B
#define PIN_FB2_BL          7U
#define PIN_FB3_BH_PORT     PORT_B
#define PIN_FB3_BH          8U
#define PIN_FB3_BL_PORT     PORT_B
#define PIN_FB3_BL          9U
#define PIN_FB4_BH_PORT     PORT_B
#define PIN_FB4_BH          10U
#define PIN_FB4_BL_PORT     PORT_B
#define PIN_FB4_BL          11U

#define PIN_INV_AH_PORT     PORT_B
#define PIN_INV_AH          14U         /* PB14 HRPWM_5_PWMA  pin 35 */
#define PIN_INV_AL_PORT     PORT_B
#define PIN_INV_AL          15U         /* PB15 HRPWM_5_PWMB  pin 36 */
#define PIN_INV_BH_PORT     PORT_C
#define PIN_INV_BH          8U          /* PC8  HRPWM_6_PWMA  pin 39 */
#define PIN_INV_BL_PORT     PORT_C
#define PIN_INV_BL          9U          /* PC9  HRPWM_6_PWMB  pin 40 */

#define PIN_IPH1            0U          /* PA0 */
#define PIN_IPH2            1U
#define PIN_IPH3            2U
#define PIN_IPH4            3U
#define PIN_VIN             6U          /* PA6 */
#define PIN_ISER            0U          /* PB0 */
#define PIN_VBUS            0U          /* PC0 */
#define PIN_VAC             1U          /* PC1 */
#define PIN_IAC             2U          /* PC2 */
#define PIN_TEMP            3U          /* PC3 */

#define PIN_DAC_OCP         4U          /* PA4 */
#define PIN_CMP1_OCP        7U          /* PA7 */
#define PIN_EMB_FAULT       5U          /* PC5 */
#define PIN_ENABLE          2U          /* PD2 */
#define PIN_LED_RUN         4U          /* PC4 */

#define ADC_CH_IPH1         0U
#define ADC_CH_IPH2         1U
#define ADC_CH_IPH3         2U
#define ADC_CH_IPH4         3U
#define ADC_CH_VIN          6U
#define ADC_CH_ISER         8U
#define ADC_CH_VBUS         10U
#define ADC_CH_VAC          11U
#define ADC_CH_IAC          12U
#define ADC_CH_TEMP         13U

static const board_pwm_pin_t k_front_a_pins[8] = {
    {PORT_C, 6U,  "HRPWM_1_PWMA", 37U},
    {PORT_C, 7U,  "HRPWM_1_PWMB", 38U},
    {PORT_A, 8U,  "HRPWM_2_PWMA", 41U},
    {PORT_A, 9U,  "HRPWM_2_PWMB", 42U},
    {PORT_A, 10U, "HRPWM_3_PWMA", 43U},
    {PORT_A, 11U, "HRPWM_3_PWMB", 44U},
    {PORT_B, 12U, "HRPWM_4_PWMA", 33U},
    {PORT_B, 13U, "HRPWM_4_PWMB", 34U},
};

static const board_pwm_pin_t k_front_b_pins[8] = {
    {PORT_B, 4U,  "TIM6_1_PWMA", 0U},
    {PORT_B, 5U,  "TIM6_1_PWMB", 0U},
    {PORT_B, 6U,  "TIM6_2_PWMA", 0U},
    {PORT_B, 7U,  "TIM6_2_PWMB", 0U},
    {PORT_B, 8U,  "TIM6_3_PWMA", 0U},
    {PORT_B, 9U,  "TIM6_3_PWMB", 0U},
    {PORT_B, 10U, "TIM6_4_PWMA", 0U},
    {PORT_B, 11U, "TIM6_4_PWMB", 0U},
};

static const board_pwm_pin_t k_inv_pins[4] = {
    {PORT_B, 14U, "HRPWM_5_PWMA", 35U},
    {PORT_B, 15U, "HRPWM_5_PWMB", 36U},
    {PORT_C, 8U,  "HRPWM_6_PWMA", 39U},
    {PORT_C, 9U,  "HRPWM_6_PWMB", 40U},
};

#endif
