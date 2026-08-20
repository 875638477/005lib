#include "adc_acq.h"
#include "app_config.h"
#include "board_pin.h"

/*
 * Target:
 *   ADC1: PA0-PA3 IPH1-4, trig HRPWM valley
 *   ADC2: PA6 VIN, PB0 ISER, PC0 VBUS, trig HRPWM1 period
 *   ADC3: PC1 VAC, PC2 IAC, PC3 TEMP
 * V1-V4 come from isolated SPI ADC, copied into sense_raw.vmod[].
 * Analog watchdog on VBUS / ISER -> EMB.
 */

static sense_raw_t s_host_raw;

void adc_acq_init(void)
{
    unsigned i;

    s_host_raw.vin = 0;
    s_host_raw.vbus = 0;
    s_host_raw.iser = (uint16_t)(ADC_CH_ISER * 0U + 2047U);
    s_host_raw.vac = 2047;
    s_host_raw.iac = 2047;
    s_host_raw.temp = 0;
    for (i = 0; i < 4; ++i) {
        s_host_raw.vmod[i] = 0;
        s_host_raw.iph[i] = 2047;
    }
    (void)k_inv_pins;
}

void adc_acq_inject_host(const sense_raw_t *raw)
{
    s_host_raw = *raw;
}

void adc_acq_read(sense_raw_t *raw)
{
    *raw = s_host_raw;
}
