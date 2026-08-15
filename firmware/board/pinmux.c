/*
 * IOC setup skeleton for HPM6284 4-phase full-bridge inverter board.
 * Link against HPM SDK (hpm_ioc_regs.h / hpm_iomux.h).
 */

#include "hpm6284_inverter_pinmux.h"

#ifdef HPM_SDK
#include "hpm_ioc_regs.h"
#include "hpm_iomux.h"
#include "hpm_gpio_drv.h"
#endif

#ifndef IOC_PAD_FUNC_CTL_ANALOG_MASK
#define IOC_PAD_FUNC_CTL_ANALOG_MASK  (1u << 8)
#endif

static void set_pwm_pad(uint32_t pad, uint32_t alt)
{
#ifdef HPM_SDK
    HPM_IOC->PAD[pad].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(alt);
    HPM_IOC->PAD[pad].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
#else
    (void)pad;
    (void)alt;
#endif
}

static void set_analog_pad(uint32_t pad)
{
#ifdef HPM_SDK
    HPM_IOC->PAD[pad].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
#else
    (void)pad;
#endif
}

void board_init_inverter_pins(void)
{
    const uint32_t ph12[] = {
        PIN_PH1_AH, PIN_PH1_AL, PIN_PH1_BH, PIN_PH1_BL,
        PIN_PH2_AH, PIN_PH2_AL, PIN_PH2_BH, PIN_PH2_BL,
        PIN_DCDC_FAULT_OC, PIN_DCDC_FAULT_AUX
    };
    const uint32_t ph34[] = {
        PIN_PH3_AH, PIN_PH3_AL, PIN_PH3_BH, PIN_PH3_BL,
        PIN_PH4_AH, PIN_PH4_AL, PIN_PH4_BH, PIN_PH4_BL
    };
    const uint32_t inv[] = {
        PIN_INV_AH, PIN_INV_AL, PIN_INV_BH, PIN_INV_BL,
        PIN_INV_FAULT_OC, PIN_INV_FAULT_AUX
    };
    const uint32_t analog[] = {
        PIN_ADC_I_PH1, PIN_ADC_I_PH2, PIN_ADC_I_PH3, PIN_ADC_I_PH4,
        PIN_ADC_I_AC, PIN_ADC_V_IN, PIN_ADC_V_BUS, PIN_ADC_V_AC,
        PIN_ADC_I_IN, PIN_ADC_NTC_MOS, PIN_ADC_NTC_XFMR,
        PIN_ADC_NTC_IGBT, PIN_ADC_P12V
    };
    uint32_t i;

    for (i = 0; i < sizeof(ph12) / sizeof(ph12[0]); i++) {
        set_pwm_pad(ph12[i], INV_ALT_PWM);
    }
    for (i = 0; i < sizeof(ph34) / sizeof(ph34[0]); i++) {
        set_pwm_pad(ph34[i], INV_ALT_PWM23);
    }
    for (i = 0; i < sizeof(inv) / sizeof(inv[0]); i++) {
        set_pwm_pad(inv[i], INV_ALT_PWM);
    }

    set_pwm_pad(PIN_FAN1, INV_ALT_PWM23);
    set_pwm_pad(PIN_FAN2, INV_ALT_PWM23);

#ifdef HPM_SDK
    HPM_IOC->PAD[PIN_UART0_TX].FUNC_CTL = IOC_PA30_FUNC_CTL_UART0_TXD;
    HPM_IOC->PAD[PIN_UART0_RX].FUNC_CTL = IOC_PA31_FUNC_CTL_UART0_RXD;
#endif

    for (i = 0; i < sizeof(analog) / sizeof(analog[0]); i++) {
        set_analog_pad(analog[i]);
    }
}
