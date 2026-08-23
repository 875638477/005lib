#ifndef PROTECT_H
#define PROTECT_H

#include <stdint.h>
#include "sense_scale.h"

typedef enum {
    PROT_OK = 0,
    PROT_VRECT_UV,
    PROT_VRECT_OV,
    PROT_VOUT_OV,
    PROT_OCP,
    PROT_OTP,
    PROT_HW_EMB
} prot_fault_t;

typedef struct {
    prot_fault_t fault;
    uint8_t latch;
    uint8_t hw_emb;
} protect_t;

void protect_init(protect_t *p);
void protect_clear(protect_t *p);
void protect_note_hw_emb(protect_t *p);
prot_fault_t protect_eval(protect_t *p, const sense_eng_t *s, uint8_t field_present);
const char *protect_fault_name(prot_fault_t f);

#endif
