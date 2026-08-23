#include "hrpwm_sr.h"
#include "app_config.h"
#include "board_pin.h"

/*
 * Target bring-up:
 *   HRPWM1/2 complementary, sync to CMP1 (coil polarity) via AOS/PLA.
 *   SR_MODE_DIODE: both PWMA/PWMB forced idle (low), Schottky/body diode.
 *   Enable SR only after Vrect > UV and polarity confirmed.
 */

uint32_t hrpwm_ns_to_ticks(uint32_t ns)
{
    return (uint32_t)(((uint64_t)ns * CPU_CLK_HZ) / 1000000000ULL);
}

void hrpwm_sr_init(hrpwm_sr_t *h)
{
    h->mode = SR_MODE_DIODE;
    h->polarity = 0U;
    h->enabled = 0U;
    h->emb_latched = 0U;
    h->deadtime_ticks = hrpwm_ns_to_ticks(DEADTIME_NS);
    (void)k_hrpwm_pins;
}

void hrpwm_sr_set_mode(hrpwm_sr_t *h, sr_mode_t mode)
{
    h->mode = mode;
}

void hrpwm_sr_enable(hrpwm_sr_t *h, uint8_t en)
{
    if (h->emb_latched || h->mode == SR_MODE_DIODE) {
        h->enabled = 0U;
        return;
    }
    h->enabled = en ? 1U : 0U;
}

void hrpwm_sr_emb_trip(hrpwm_sr_t *h)
{
    h->emb_latched = 1U;
    h->enabled = 0U;
}

void hrpwm_sr_emb_clear(hrpwm_sr_t *h)
{
    h->emb_latched = 0U;
}
