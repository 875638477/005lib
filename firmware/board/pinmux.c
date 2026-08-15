/*
 * IOC setup skeleton for HPM6284 inverter board.
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

static void set_pwm_pad(uint32_t pad)
{
#ifdef HPM_SDK
    HPM_IOC->PAD[pad].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(INV_ALT_PWM);
    HPM_IOC->PAD[pad].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
#else
    (void)pad;
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
    set_pwm_pad(PIN_PSFB_AH);
    set_pwm_pad(PIN_PSFB_AL);
    set_pwm_pad(PIN_PSFB_BH);
    set_pwm_pad(PIN_PSFB_BL);
    set_pwm_pad(PIN_PSFB_FAULT_OC);
    set_pwm_pad(PIN_PSFB_FAULT_AUX);

    set_pwm_pad(PIN_INV_AH);
    set_pwm_pad(PIN_INV_AL);
    set_pwm_pad(PIN_INV_BH);
    set_pwm_pad(PIN_INV_BL);
    set_pwm_pad(PIN_INV_FAULT_OC);
    set_pwm_pad(PIN_INV_FAULT_AUX);

#ifdef HPM_SDK
    HPM_IOC->PAD[PIN_FAN1].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(INV_ALT_PWM2);
    HPM_IOC->PAD[PIN_FAN2].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(INV_ALT_PWM2);
    HPM_IOC->PAD[PIN_UART0_TX].FUNC_CTL = IOC_PA30_FUNC_CTL_UART0_TXD;
    HPM_IOC->PAD[PIN_UART0_RX].FUNC_CTL = IOC_PA31_FUNC_CTL_UART0_RXD;
#endif

    set_analog_pad(PIN_ADC_I_PRI);
    set_analog_pad(PIN_ADC_I_AC);
    set_analog_pad(PIN_ADC_I_IN);
    set_analog_pad(PIN_ADC_I_BUS);
    set_analog_pad(PIN_ADC_V_IN);
    set_analog_pad(PIN_ADC_V_BUS);
    set_analog_pad(PIN_ADC_V_AC);
    set_analog_pad(PIN_ADC_V_N);
    set_analog_pad(PIN_ADC_NTC_MOS);
    set_analog_pad(PIN_ADC_NTC_XFMR);
    set_analog_pad(PIN_ADC_NTC_IGBT);
    set_analog_pad(PIN_ADC_P12V);
    set_analog_pad(PIN_ADC_P3V3);
}
