#include "protect.h"
#include "app_config.h"

void protect_init(protect_t *p)
{
    p->state = PROT_ST_IDLE;
    p->fault = PROT_OK;
    p->iout_over_us = 0U;
    p->latch = 0U;
    p->hw_emb = 0U;
}

void protect_clear(protect_t *p)
{
    if (p->fault == PROT_SCP) {
        return;
    }
    protect_init(p);
}

void protect_note_hw_emb(protect_t *p)
{
    p->hw_emb = 1U;
}

static prot_fault_t classify(const sense_eng_t *s)
{
    unsigned i;

    if (s->vout_mv >= OVP_SOFT_MV) {
        return PROT_OVP;
    }
    if (s->vin_mv >= VIN_OVLO_MV) {
        return PROT_VIN_OV;
    }
    if (s->vin_mv > 0 && s->vin_mv < VIN_UVLO_MV) {
        return PROT_VIN_UV;
    }
    if (s->temp_c >= OTP_C) {
        return PROT_OTP;
    }
    if (s->vout_mv < SCP_VOUT_MV && s->iout_ma > SCP_IOUT_MA) {
        return PROT_SCP;
    }
    for (i = 0; i < PHASE_COUNT; ++i) {
        if (s->iph_ma[i] > OCP_IPH_MA) {
            return PROT_OCP_PH;
        }
    }
    return PROT_OK;
}

prot_fault_t protect_eval(protect_t *p, const sense_eng_t *s, uint32_t dt_us)
{
    prot_fault_t f;

    if (p->latch) {
        p->state = PROT_ST_FAULT;
        return p->fault;
    }
    if (p->hw_emb) {
        p->fault = PROT_HW_EMB;
        p->latch = 1U;
        p->state = PROT_ST_FAULT;
        return p->fault;
    }

    f = classify(s);
    if (s->iout_ma > OCP_IOUT_MA) {
        p->iout_over_us += dt_us;
        if (p->iout_over_us >= OCP_IOUT_HOLD_US) {
            f = PROT_OCP_OUT;
        }
    } else {
        p->iout_over_us = 0U;
    }

    if (f != PROT_OK) {
        p->fault = f;
        p->latch = 1U;
        p->state = PROT_ST_FAULT;
        return f;
    }

    if (p->state == PROT_ST_IDLE) {
        p->state = PROT_ST_SOFTSTART;
    } else if (p->state == PROT_ST_SOFTSTART) {
        if (s->vout_mv >= (VOUT_NOM_MV - 300)) {
            p->state = PROT_ST_RUN;
        }
    }
    return PROT_OK;
}

const char *protect_fault_name(prot_fault_t f)
{
    switch (f) {
    case PROT_OK:       return "OK";
    case PROT_OCP_PH:   return "OCP_PH";
    case PROT_OCP_OUT:  return "OCP_OUT";
    case PROT_SCP:      return "SCP";
    case PROT_OVP:      return "OVP";
    case PROT_VIN_OV:   return "VIN_OV";
    case PROT_VIN_UV:   return "VIN_UV";
    case PROT_OTP:      return "OTP";
    case PROT_HW_EMB:   return "HW_EMB";
    default:            return "?";
    }
}
