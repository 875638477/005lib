#include "protect.h"
#include "app_config.h"

void protect_init(protect_t *p)
{
    p->fault = PROT_OK;
    p->latch = 0U;
    p->hw_emb = 0U;
}

void protect_clear(protect_t *p)
{
    if (!p->hw_emb) {
        p->fault = PROT_OK;
        p->latch = 0U;
    }
}

void protect_note_hw_emb(protect_t *p)
{
    p->hw_emb = 1U;
    p->latch = 1U;
    p->fault = PROT_HW_EMB;
}

prot_fault_t protect_eval(protect_t *p, const sense_eng_t *s, uint8_t field_present)
{
    if (p->latch) {
        return p->fault;
    }
    if (s->vrect_mv > VRECT_OVP_MV) {
        p->fault = PROT_VRECT_OV;
    } else if (s->vout_mv > VOUT_OVP_MV) {
        p->fault = PROT_VOUT_OV;
    } else if (s->iout_ma > IOUT_OCP_MA) {
        p->fault = PROT_OCP;
    } else if (s->ntc_coil_c >= OTP_TRIP_C || s->ntc_sr_c >= OTP_TRIP_C) {
        p->fault = PROT_OTP;
    } else if (field_present && s->vrect_mv < VRECT_UV_MV) {
        p->fault = PROT_VRECT_UV;
    } else {
        p->fault = PROT_OK;
        return PROT_OK;
    }
    p->latch = 1U;
    return p->fault;
}

const char *protect_fault_name(prot_fault_t f)
{
    switch (f) {
    case PROT_OK:       return "ok";
    case PROT_VRECT_UV: return "vrect_uv";
    case PROT_VRECT_OV: return "vrect_ov";
    case PROT_VOUT_OV:  return "vout_ov";
    case PROT_OCP:      return "ocp";
    case PROT_OTP:      return "otp";
    case PROT_HW_EMB:   return "hw_emb";
    default:            return "unknown";
    }
}
