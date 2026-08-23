#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "app_config.h"
#include "board_pin.h"
#include "wpt_math.h"
#include "hrpwm_inv.h"
#include "sense_scale.h"
#include "protect.h"
#include "power_ctrl.h"
#include "wpc_sm.h"
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
    expect(k_hrpwm_pins[0].port == PORT_C && k_hrpwm_pins[0].pin == 6U, "legA HS PC6");
    expect(k_hrpwm_pins[1].port == PORT_C && k_hrpwm_pins[1].pin == 7U, "legA LS PC7");
    expect(k_hrpwm_pins[2].port == PORT_A && k_hrpwm_pins[2].pin == 8U, "legB HS PA8");
    expect(k_hrpwm_pins[4].port == PORT_A && k_hrpwm_pins[4].pin == 10U, "VBRG HS PA10");
    expect(PIN_ICOIL == 0U && PIN_VIN == 0U && PIN_CMP1_OCP == 7U, "ADC/CMP pins");
    expect(k_hrpwm_pins[0].lqfp64 == 37U && k_hrpwm_pins[5].lqfp64 == 44U, "LQFP64 numbers");
}

static void test_resonant(void)
{
    uint32_t fr = wpt_fres_hz(FRES_LTX_NH, FRES_CS_NF);
    uint32_t p127 = wpt_hrpwm_period_ticks(127700U);
    uint32_t p110 = wpt_hrpwm_period_ticks(110000U);
    uint32_t p205 = wpt_hrpwm_period_ticks(205000U);
    uint32_t d_fsk = wpt_fsk_delta_ticks(127700U, 1000U);

    expect(fr > 99000U && fr < 102000U, "fres ~ 100 kHz");
    expect(p127 == 120000000U / 127700U, "127.7 kHz period");
    expect(p110 == 120000000U / 110000U, "110 kHz period");
    expect(p205 == 120000000U / 205000U, "205 kHz period");
    expect(p110 > p127 && p127 > p205, "period vs freq");
    expect(d_fsk > 0U && d_fsk < 20U, "FSK 1 kHz is a few coarse ticks");
    expect(wpt_hrpwm_period_ticks(50000U) == 120000000U / 110000U, "clamp min 110 kHz");
}

static void test_power_class(void)
{
    expect(P_CONTRACT_MW == 20000, "default contract 20 W");
    expect(wpt_contract_mw_for_vin(20000) == 20000, "20 V still 20 W when compiled 20");
    expect(wpt_contract_mw_for_vin(10000) == 20000, "10 V contract 20 W");
    expect(wpt_pin_cap_mw(20000) == (20000 * 110) / 80, "Pin cap 20 W / 80%");
    expect(wpt_fod_trip(1200, FOD_PLOSS_MW) == 1, "FOD trip 1.2 W");
    expect(wpt_fod_trip(200, FOD_PLOSS_MW) == 0, "FOD hold 0.2 W");
    expect(fabsf(wpt_q_from_decay(1.0f, expf(-3.14159265f / 80.0f), 1U) - 80.0f) < 1.0f,
           "Q ~ 80 from one-cycle decay");
}

static void test_sense_protect(void)
{
    sense_raw_t raw;
    sense_eng_t eng;
    protect_t p;
    uint16_t lsb12;

    memset(&raw, 0, sizeof(raw));
    lsb12 = sense_mv_to_vbrg_lsb(12000);
    expect(abs(sense_vbrg_lsb_to_mv(lsb12) - 12000) < 20, "12 V VBRG reconstruct");
    expect(abs(sense_icoil_lsb_to_ma(sense_ma_to_icoil_lsb(3000)) - 3000) < 20, "3 A Icoil");

    raw.vin = (uint16_t)(((int64_t)12000 * ADC_MAX_LSB) / VIN_ADC_FS_MV);
    raw.vbrg = lsb12;
    raw.iin = (uint16_t)(((int64_t)2000 * ADC_MAX_LSB) / IIN_FS_MA);
    raw.icoil = sense_ma_to_icoil_lsb(3000);
    raw.ntc_coil = (uint16_t)((25 * ADC_MAX_LSB) / 150);
    raw.ntc_mos = raw.ntc_coil;
    sense_raw_to_eng(&raw, &eng);
    expect(abs(eng.vin_mv - 12000) < 40, "eng VIN 12 V");
    expect(abs(eng.iin_ma - 2000) < 20, "eng IIN 2 A");
    expect(abs(eng.ptx_mw - 24000) < 200, "Pin ~ 24 W");

    protect_init(&p);
    expect(protect_eval(&p, &eng) == PROT_OK, "normal 20 W sense OK");

    eng.icoil_ma = ICOIL_OCP_MA + 100;
    expect(protect_eval(&p, &eng) == PROT_OCP_COIL, "coil OCP");
    expect(p.latch == 1U, "OCP latched");

    protect_init(&p);
    eng.icoil_ma = 1000;
    eng.vin_mv = 7000;
    expect(protect_eval(&p, &eng) == PROT_VIN_UV, "UVLO");

    protect_init(&p);
    eng.vin_mv = 12000;
    eng.ntc_coil_c = 110;
    expect(protect_eval(&p, &eng) == PROT_OTP, "coil OTP");
    expect(strcmp(protect_fault_name(PROT_FOD), "fod") == 0, "fault name");
}

static void test_hrpwm_cep(void)
{
    hrpwm_inv_t inv;
    power_ctrl_t pwr;
    int32_t v0;

    hrpwm_inv_init(&inv);
    expect(inv.fsw_hz == 127700U, "default 127.7 kHz");
    expect(inv.phase_deg == 180U && inv.mode == INV_MODE_FULL, "full bridge 180");
    expect(inv.period_ticks == wpt_hrpwm_period_ticks(127700U), "period match");
    expect(inv.deadtime_ticks == hrpwm_ns_to_ticks(70U), "70 ns deadtime");

    hrpwm_inv_set_freq(&inv, 205000U);
    expect(inv.fsw_hz == 205000U, "set 205 kHz");
    hrpwm_inv_emb_trip(&inv);
    hrpwm_inv_enable(&inv, 1);
    expect(inv.enabled == 0U && inv.emb_latched == 1U, "EMB blocks enable");

    power_ctrl_init(&pwr);
    power_ctrl_clamp_for_vin(&pwr, 12000);
    v0 = pwr.vbrg_mv;
    power_ctrl_apply_cep(&pwr, 5);
    expect(pwr.vbrg_mv > v0, "CEP+ raises VBRG");
    v0 = pwr.vbrg_mv;
    power_ctrl_apply_cep(&pwr, -5);
    expect(pwr.vbrg_mv < v0, "CEP- lowers VBRG");

    pwr.vbrg_mv = pwr.vbrg_max_mv;
    pwr.fsw_hz = 127700U;
    power_ctrl_apply_cep(&pwr, 8);
    expect(pwr.vbrg_sat == 1U, "VBRG sat");
    expect(pwr.fsw_hz < 127700U, "after sat, lower freq to add power");
}

static void ask_push_bit(ask_demod_t *d, uint8_t *level, uint8_t bit)
{
    *level ^= 1U;
    ask_demod_halfbit(d, *level);
    if (bit == 0U) {
        *level ^= 1U;
    }
    ask_demod_halfbit(d, *level);
}

static void ask_push_byte(ask_demod_t *d, uint8_t *level, uint8_t byte)
{
    int i;

    for (i = 7; i >= 0; --i) {
        ask_push_bit(d, level, (uint8_t)((byte >> i) & 1U));
    }
}

static void test_ask(void)
{
    ask_demod_t d;
    uint8_t level = 0U;
    uint8_t pkt[3];
    unsigned i;

    pkt[0] = 0x01;
    pkt[1] = 0x80;
    pkt[2] = ask_checksum(pkt, 2U);
    expect(ask_packet_ok(pkt, 3U), "ss packet checksum");

    ask_demod_init(&d);
    for (i = 0; i < ASK_PREAMBLE_ONES; ++i) {
        ask_push_bit(&d, &level, 1U);
    }
    ask_push_byte(&d, &level, pkt[0]);
    ask_push_byte(&d, &level, pkt[1]);
    ask_push_byte(&d, &level, pkt[2]);
    expect(d.packet_ready == 1U, "ASK packet ready");
    expect(d.nbytes == 3U && d.bytes[0] == 0x01 && d.bytes[1] == 0x80, "SS header+value");
}

static void test_wpc_sm(void)
{
    wpc_sm_t s;
    uint8_t ss = 0x80;
    uint8_t cep = (uint8_t)(int8_t)3;
    uint8_t rpp = 40;

    wpc_sm_init(&s);
    expect(s.state == WPC_SEL, "start selection");
    s.state = WPC_PING;
    wpc_sm_on_packet(&s, 0x01, &ss, 1U);
    expect(s.state == WPC_ID && s.ss_pkt == 1U, "SS -> ID");
    wpc_sm_on_packet(&s, 0x71, NULL, 0U);
    wpc_sm_on_packet(&s, 0x51, NULL, 0U);
    expect(s.state == WPC_NEGO, "config -> nego");
    wpc_sm_on_packet(&s, 0x03, &cep, 1U);
    expect(s.state == WPC_PT && s.last_cep == 3, "CEP -> PT");
    wpc_sm_on_packet(&s, 0x04, &rpp, 1U);
    expect(s.last_prx_mw == 20000, "RPP 40 * 500 mW");
    wpc_sm_tick_ms(&s, 1600U);
    expect(s.state == WPC_SEL, "PT timeout back to selection");
    expect(strcmp(wpc_state_name(WPC_PT), "power_transfer") == 0, "state name");
}

int main(void)
{
    test_pins();
    test_resonant();
    test_power_class();
    test_sense_protect();
    test_hrpwm_cep();
    test_ask();
    test_wpc_sm();

    if (g_fail) {
        fprintf(stderr, "test_wpt FAILED\n");
        return 1;
    }
    printf("test_wpt OK (20 W profile)\n");
    return 0;
}
