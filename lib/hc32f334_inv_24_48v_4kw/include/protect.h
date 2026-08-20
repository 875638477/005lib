#ifndef PROTECT_H
#define PROTECT_H

#include <stdint.h>
#include "sense_scale.h"

typedef enum {
    PROT_OK = 0,
    PROT_VIN_UV,
    PROT_VIN_OV,
    PROT_VBUS_OV,
    PROT_VBUS_UV,
    PROT_VMOD_OV,
    PROT_VMOD_UNBAL,
    PROT_OCP_PH,
    PROT_OCP_SER,
    PROT_OCP_AC,
    PROT_OTP,
    PROT_HW_EMB
} prot_fault_t;

typedef enum {
    PROT_ST_IDLE = 0,
    PROT_ST_PRECHARGE,
    PROT_ST_DCDC_SS,
    PROT_ST_INV_SS,
    PROT_ST_RUN,
    PROT_ST_FAULT
} prot_state_t;

typedef struct {
    prot_state_t state;
    prot_fault_t fault;
    uint32_t unbal_ms;
    uint32_t iac_over_ms;
    uint8_t latch;
    uint8_t hw_emb;
} protect_t;

void protect_init(protect_t *p);
void protect_clear(protect_t *p);
void protect_note_hw_emb(protect_t *p);
prot_fault_t protect_eval(protect_t *p, const sense_eng_t *s, uint32_t dt_us);
const char *protect_fault_name(prot_fault_t f);

#endif
