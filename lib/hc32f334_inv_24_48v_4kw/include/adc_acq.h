#ifndef ADC_ACQ_H
#define ADC_ACQ_H

#include "sense_scale.h"

void adc_acq_init(void);
void adc_acq_read(sense_raw_t *raw);
void adc_acq_inject_host(const sense_raw_t *raw);

#endif
