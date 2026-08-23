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

prot_fault_t protect_eval(protect_t *p, const sense_eng_t *s)
{
    if (p->latch) {
        return p->fault;
    }
    if (s->vin_mv < VIN_UVLO_MV) {
        p->fault = PROT_VIN_UV;
    } else if (s->vin_mv > VIN_OVLO_MV) {
        p->fault = PROT_VIN_OV;
    } else if (s->vbrg_mv > (VBRG_MAX_MV + 1500)) {
        p->fault = PROT_VBRG_OV;
    } else if (s->icoil_ma > ICOIL_OCP_MA) {
        p->fault = PROT_OCP_COIL;
    } else if (s->iin_ma > IIN_OCP_MA) {
        p->fault = PROT_OCP_IN;
    } else if (s->ntc_coil_c >= OTP_TRIP_C || s->ntc_mos_c >= OTP_TRIP_C) {
        p->fault = PROT_OTP;
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
    case PROT_OCP_COIL: return "ocp_coil";
    case PROT_OCP_IN:   return "ocp_in";
    case PROT_VIN_OV:   return "vin_ov";
    case PROT_VIN_UV:   return "vin_uv";
    case PROT_VBRG_OV:  return "vbrg_ov";
    case PROT_OTP:      return "otp";
    case PROT_FOD:      return "fod";
    case PROT_COMM:     return "comm";
    case PROT_HW_EMB:   return "hw_emb";
    default:            return "unknown";
    }
}
