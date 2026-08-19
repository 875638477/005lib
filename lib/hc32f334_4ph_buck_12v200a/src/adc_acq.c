#include "adc_acq.h"
#include "board_pin.h"

/*
 * Target: ADC1 seq A = PA0/PA1 (IPH1/2) trig HRPWM1/2 valley
 *         ADC2 seq A = PA2/PA3 (IPH3/4) trig HRPWM3/4 valley
 *         ADC3 seq A = PA6/PC0/PB0 (VOUT/VIN/IOUT) trig HRPWM1 period
 * Analog watchdog on VOUT and IOUT -> EMB event.
 */

static sense_raw_t s_host_raw;

void adc_acq_init(void)
{
    s_host_raw.vin = 0;
    s_host_raw.vout = 0;
    s_host_raw.iout = 0;
    s_host_raw.temp = 0;
    s_host_raw.iph[0] = s_host_raw.iph[1] = s_host_raw.iph[2] = s_host_raw.iph[3] = 0;
    (void)ADC_UNIT_IPH_A;
    (void)ADC_UNIT_IPH_B;
    (void)ADC_UNIT_BUS;
}

void adc_acq_inject_host(const sense_raw_t *raw)
{
    s_host_raw = *raw;
}

void adc_acq_read(sense_raw_t *raw)
{
    *raw = s_host_raw;
}
