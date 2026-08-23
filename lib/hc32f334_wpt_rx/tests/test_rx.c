#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "app_config.h"
#include "board_pin.h"
#include "rx_math.h"
#include "hrpwm_sr.h"
#include "hrpwm_buck.h"
#include "sense_scale.h"
#include "protect.h"
#include "wpc_sm_rx.h"
#include "ask_mod.h"
#include "ask_demod.h"

static int g_fail;

static void expect(int ok, const char *msg)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_fail = 1;
    }
}

static void test_pins(void)
{
    expect(k_hrpwm_pins[0].port == PORT_C && k_hrpwm_pins[0].pin == 6U, "SR A HS PC6");
    expect(k_hrpwm_pins[2].port == PORT_A && k_hrpwm_pins[2].pin == 8U, "SR B HS PA8");
    expect(k_hrpwm_pins[4].port == PORT_A && k_hrpwm_pins[4].pin == 10U, "Buck HS PA10");
    expect(PIN_ASK == 4U && PIN_VRECT == 0U && PIN_CMP1_SR == 7U, "ASK/Vrect/CMP");
}

static void test_resonant_cep(void)
{
    uint32_t fr = rx_fres_hz(FRES_LRX_NH, FRES_CS_NF);
    int16_t up = rx_cep_from_vrect(12000, VRECT_TGT_MV);
    int16_t dn = rx_cep_from_vrect(16000, VRECT_TGT_MV);
    int32_t prx = rx_prx_mw(12000, 1670);

    expect(fr > 99000U && fr < 102000U, "RX fres ~ 100 kHz");
    expect(up > 0, "low Vrect -> CEP+");
    expect(dn < 0, "high Vrect -> CEP-");
    expect(rx_cep_from_vrect(VRECT_TGT_MV, VRECT_TGT_MV) == 0, "on target CEP 0");
    expect(abs(prx - 20040) < 50, "12 V * 1.67 A ~ 20 W");
    expect(rx_rpp8_from_mw(20000) == 40U, "RPP8 20 W = 40");
    expect(rx_rpp16_from_mw(50000) == 50000U, "RPP16 50 W");
}

static void test_sense_protect(void)
{
    sense_raw_t raw;
    sense_eng_t eng;
    protect_t p;
    uint16_t lsb12;

    memset(&raw, 0, sizeof(raw));
    lsb12 = sense_mv_to_vout_lsb(12000);
    expect(abs(sense_vout_lsb_to_mv(lsb12) - 12000) < 20, "12 V reconstruct");

    raw.vout = lsb12;
    raw.vrect = (uint16_t)(((int64_t)14000 * ADC_MAX_LSB) / VRECT_ADC_FS_MV);
    raw.iout = sense_ma_to_iout_lsb(1670);
    raw.ntc_coil = (uint16_t)((30 * ADC_MAX_LSB) / 150);
    raw.ntc_sr = raw.ntc_coil;
    sense_raw_to_eng(&raw, &eng);
    expect(abs(eng.vout_mv - 12000) < 30, "eng Vout");
    expect(abs(eng.vrect_mv - 14000) < 40, "eng Vrect");
    expect(abs(eng.prx_mw - 20000) < 200, "eng Prx 20 W");

    protect_init(&p);
    expect(protect_eval(&p, &eng, 1U) == PROT_OK, "normal OK");

    eng.vrect_mv = VRECT_OVP_MV + 100;
    expect(protect_eval(&p, &eng, 1U) == PROT_VRECT_OV, "Vrect OVP");
    expect(p.latch == 1U, "OVP latched");

    protect_init(&p);
    eng.vrect_mv = 14000;
    eng.iout_ma = IOUT_OCP_MA + 50;
    expect(protect_eval(&p, &eng, 1U) == PROT_OCP, "Iout OCP");
}

static void test_sr_buck(void)
{
    hrpwm_sr_t sr;
    hrpwm_buck_t buck;

    hrpwm_sr_init(&sr);
    expect(sr.mode == SR_MODE_DIODE, "default diode");
    hrpwm_sr_enable(&sr, 1);
    expect(sr.enabled == 0U, "diode mode refuses SR PWM");
    hrpwm_sr_set_mode(&sr, SR_MODE_SYNC);
    hrpwm_sr_enable(&sr, 1);
    expect(sr.enabled == 1U, "sync enable");
    hrpwm_sr_emb_trip(&sr);
    hrpwm_sr_enable(&sr, 1);
    expect(sr.enabled == 0U, "EMB blocks SR");

    hrpwm_buck_init(&buck);
    expect(buck.period_ticks == CPU_CLK_HZ / 200000U, "200 kHz buck");
    hrpwm_buck_set_duty(&buck, 0.80f);
    expect(buck.duty == 0.0f, "duty ignored while disabled");
    hrpwm_buck_enable(&buck, 1);
    hrpwm_buck_set_duty(&buck, 0.80f);
    expect(fabsf(buck.duty - 0.80f) < 1e-4f, "duty 0.80");
    hrpwm_buck_set_duty(&buck, 1.20f);
    expect(buck.duty <= 0.92f, "duty clamp");
}

static void test_ask_pair(void)
{
    ask_demod_t d;
    uint8_t levels[ASK_MOD_MAX_LEVELS];
    uint8_t payload[2] = {0x01, 0x80};
    unsigned n, i;

    n = ask_mod_encode(levels, ASK_MOD_MAX_LEVELS, payload, 2U);
    expect(n > 20U, "encoded levels");
    expect(ask_mod_xor(payload, 2U) == (uint8_t)(0x01 ^ 0x80), "xor");

    ask_demod_init(&d);
    for (i = 0; i < n; ++i) {
        ask_demod_halfbit(&d, levels[i]);
    }
    expect(d.packet_ready == 1U, "TX demod sees RX packet");
    expect(d.nbytes == 3U && d.bytes[0] == 0x01 && d.bytes[1] == 0x80, "SS payload");
    expect(ask_packet_ok(d.bytes, d.nbytes), "checksum ok");
}

static void test_sm(void)
{
    wpc_sm_rx_t s;

    wpc_sm_rx_init(&s);
    expect(s.state == RX_IDLE, "idle");
    wpc_sm_rx_on_field(&s, 1U);
    expect(s.state == RX_SS, "field -> SS");
    wpc_sm_rx_update_cep(&s, 5, 5000);
    expect(s.state == RX_ID && s.ss_sent == 1U, "first update -> ID");
    wpc_sm_rx_update_cep(&s, 3, 20000);
    expect(s.state == RX_PT && s.prx_mw == 20000, "then PT");
    wpc_sm_rx_on_field(&s, 0U);
    expect(s.state == RX_IDLE, "lost field -> idle");
    expect(strcmp(wpc_sm_rx_name(RX_PT), "power_transfer") == 0, "name");
}

int main(void)
{
    test_pins();
    test_resonant_cep();
    test_sense_protect();
    test_sr_buck();
    test_ask_pair();
    test_sm();

    if (g_fail) {
        fprintf(stderr, "test_rx FAILED\n");
        return 1;
    }
    printf("test_rx OK (20 W RX + ASK pair)\n");
    return 0;
}
