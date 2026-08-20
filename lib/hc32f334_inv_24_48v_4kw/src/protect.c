#include "protect.h"
#include "app_config.h"

void protect_init(protect_t *p)
{
    p->state = PROT_ST_IDLE;
    p->fault = PROT_OK;
    p->unbal_ms = 0U;
    p->iac_over_ms = 0U;
    p->latch = 0U;
    p->hw_emb = 0U;
}

void protect_clear(protect_t *p)
{
    protect_init(p);
}

void protect_note_hw_emb(protect_t *p)
{
    p->hw_emb = 1U;
}

static int32_t iabs32(int32_t x)
{
    return (x < 0) ? -x : x;
}

static prot_fault_t classify(protect_t *p, const sense_eng_t *s, uint32_t dt_us)
{
    unsigned i;
    int32_t avg;
    int32_t lim;
    uint32_t dt_ms;

    dt_ms = dt_us / 1000U;

    if (s->vin_mv >= VIN_OVLO_MV) {
        return PROT_VIN_OV;
    }
    if (s->vin_mv > 0 && s->vin_mv < VIN_UVLO_MV) {
        return PROT_VIN_UV;
    }
    if (s->vbus_mv >= VBUS_OV_SOFT_MV) {
        return PROT_VBUS_OV;
    }
    if (s->temp_c >= OTP_C) {
        return PROT_OTP;
    }
    if (iabs32(s->iser_ma) > ISER_OCP_MA) {
        return PROT_OCP_SER;
    }
    for (i = 0; i < PHASE_COUNT; ++i) {
        if (s->iph_ma[i] > IPH_OCP_MA) {
            return PROT_OCP_PH;
        }
        if (s->vmod_mv[i] > VMOD_OV_MV) {
            return PROT_VMOD_OV;
        }
    }

    avg = s->vbus_mv / (int32_t)PHASE_COUNT;
    lim = (avg * VMOD_UNBAL_PCT) / 100;
    if (lim < 5000) {
        lim = 5000;
    }
    if (s->vbus_mv > 80000) {
        unsigned unbal = 0U;
        for (i = 0; i < PHASE_COUNT; ++i) {
            if (iabs32(s->vmod_mv[i] - avg) > lim) {
                unbal = 1U;
            }
        }
        if (unbal) {
            p->unbal_ms += dt_ms;
            if (p->unbal_ms >= VMOD_UNBAL_MS) {
                return PROT_VMOD_UNBAL;
            }
        } else {
            p->unbal_ms = 0U;
        }
    }

    if (iabs32(s->iac_ma) > IAC_SURGE_MA) {
        p->iac_over_ms += dt_ms;
        if (p->iac_over_ms >= 1000U) {
            return PROT_OCP_AC;
        }
    } else {
        p->iac_over_ms = 0U;
    }

    if ((p->state == PROT_ST_RUN || p->state == PROT_ST_INV_SS) &&
        s->vbus_mv < VBUS_UV_MV) {
        return PROT_VBUS_UV;
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

    f = classify(p, s, dt_us);
    if (f != PROT_OK) {
        p->fault = f;
        p->latch = 1U;
        p->state = PROT_ST_FAULT;
        return f;
    }

    if (p->state == PROT_ST_IDLE) {
        p->state = PROT_ST_PRECHARGE;
    } else if (p->state == PROT_ST_PRECHARGE) {
        if (s->vin_mv >= VIN_MIN_MV) {
            p->state = PROT_ST_DCDC_SS;
        }
    } else if (p->state == PROT_ST_DCDC_SS) {
        if (s->vbus_mv >= (VBUS_NOM_MV - 20000)) {
            p->state = PROT_ST_INV_SS;
        }
    } else if (p->state == PROT_ST_INV_SS) {
        if (s->vbus_mv >= (VBUS_NOM_MV - 20000)) {
            p->state = PROT_ST_RUN;
        }
    }
    return PROT_OK;
}

const char *protect_fault_name(prot_fault_t f)
{
    switch (f) {
    case PROT_OK:         return "OK";
    case PROT_VIN_UV:     return "VIN_UV";
    case PROT_VIN_OV:     return "VIN_OV";
    case PROT_VBUS_OV:    return "VBUS_OV";
    case PROT_VBUS_UV:    return "VBUS_UV";
    case PROT_VMOD_OV:    return "VMOD_OV";
    case PROT_VMOD_UNBAL: return "VMOD_UNBAL";
    case PROT_OCP_PH:     return "OCP_PH";
    case PROT_OCP_SER:    return "OCP_SER";
    case PROT_OCP_AC:     return "OCP_AC";
    case PROT_OTP:        return "OTP";
    case PROT_HW_EMB:     return "HW_EMB";
    default:              return "?";
    }
}
