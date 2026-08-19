#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "app_config.h"
#include "board_pin.h"
#include "csa_ina241.h"
#include "sense_scale.h"
#include "control_loop.h"
#include "protect.h"
#include "hrpwm_4ph.h"
#include "adc_acq.h"

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
    expect(k_hrpwm_pins[0].port == PORT_C && k_hrpwm_pins[0].pin == 6U, "PH1 HS PC6");
    expect(k_hrpwm_pins[1].port == PORT_C && k_hrpwm_pins[1].pin == 7U, "PH1 LS PC7");
    expect(k_hrpwm_pins[2].port == PORT_A && k_hrpwm_pins[2].pin == 8U, "PH2 HS PA8");
    expect(k_hrpwm_pins[4].port == PORT_A && k_hrpwm_pins[4].pin == 10U, "PH3 HS PA10");
    expect(k_hrpwm_pins[6].port == PORT_B && k_hrpwm_pins[6].pin == 12U, "PH4 HS PB12");
    expect(PIN_IPH1 == 0U && PIN_VOUT == 6U && PIN_IOUT == 0U, "ADC pins");
    expect(PIN_CMP1_OCP == 7U && PIN_DAC_OCP == 4U, "CMP/DAC pins");
}

static void test_csa_ina241(void)
{
    float v50 = csa_iph_volts(50.0f);
    float i50 = csa_iph_amp(v50);
    float v0 = csa_iph_volts(0.0f);

    expect(fabsf(v0 - 1.65f) < 1e-4f, "INA241 midref 1.65 V");
    expect(fabsf(v50 - 2.15f) < 1e-3f, "50 A -> 2.15 V");
    expect(fabsf(i50 - 50.0f) < 0.05f, "roundtrip 50 A");
    expect(csa_ocp_dac_lsb(OCP_IPH_MA) > 2500U, "OCP DAC above midscale");
    expect(strcmp(CSA_PART_NAME, "INA241A2") == 0, "CSA PN");
}

static void test_sense(void)
{
    sense_raw_t raw;
    sense_eng_t eng;
    uint16_t lsb12;
    uint16_t lsb50;

    memset(&raw, 0, sizeof(raw));
    lsb12 = sense_mv_to_vout_lsb(12000);
    expect(lsb12 > 2900U && lsb12 < 3050U, "12 V ADC LSB window");
    expect(abs(sense_vout_lsb_to_mv(lsb12) - 12000) < 20, "12 V reconstruct");

    lsb50 = sense_ma_to_iph_lsb(50000);
    expect(abs(sense_iph_lsb_to_ma(lsb50) - 50000) < 200, "50 A IPH reconstruct");

    raw.vout = lsb12;
    raw.vin = (uint16_t)(((int64_t)48000 * ADC_MAX_LSB) / VIN_ADC_FS_MV);
    raw.iout = (uint16_t)(((int64_t)200000 * ADC_MAX_LSB) / IOUT_HALL_FS_MA);
    raw.iph[0] = raw.iph[1] = raw.iph[2] = raw.iph[3] = lsb50;
    sense_raw_to_eng(&raw, &eng);
    expect(abs(eng.vout_mv - 12000) < 30, "eng VOUT");
    expect(abs(eng.vin_mv - 48000) < 80, "eng VIN 48 V");
    expect(abs(eng.iout_ma - 200000) < 200, "eng IOUT 200 A");
    expect(abs(eng.iph_ma[0] - 50000) < 200, "eng IPH1");
}

static void test_hrpwm(void)
{
    hrpwm_4ph_t h;
    float duty[4] = {0.40f, 0.40f, 0.40f, 0.40f};

    hrpwm_4ph_init(&h);
    expect(h.period == 600U, "200 kHz period @ 120 MHz");
    expect(h.phase_ticks[0] == 0U, "0 deg");
    expect(h.phase_ticks[1] == 150U, "90 deg");
    expect(h.phase_ticks[2] == 300U, "180 deg");
    expect(h.phase_ticks[3] == 450U, "270 deg");
    expect(h.deadtime_ticks == 9U || h.deadtime_ticks == 10U, "80 ns deadtime ticks");

    hrpwm_4ph_enable(&h, 1);
    hrpwm_4ph_apply_duty(&h, duty);
    expect(fabsf(h.duty[2] - 0.40f) < 1e-6f, "duty applied");
    hrpwm_4ph_emb_trip(&h);
    expect(h.emb_latched == 1U && h.enabled == 0U && h.duty[0] == 0.0f, "EMB zeros PWM");
    hrpwm_4ph_enable(&h, 1);
    expect(h.enabled == 0U, "EMB blocks re-enable");
}

static void fill_ok(sense_eng_t *s, int32_t vout_mv, int32_t iout_ma)
{
    unsigned i;

    s->vin_mv = 48000;
    s->vout_mv = vout_mv;
    s->iout_ma = iout_ma;
    s->temp_c = 45;
    for (i = 0; i < 4; ++i) {
        s->iph_ma[i] = iout_ma / 4;
    }
}

static void test_protect(void)
{
    protect_t p;
    sense_eng_t s;

    protect_init(&p);
    fill_ok(&s, 12000, 20000);
    expect(protect_eval(&p, &s, 20U) == PROT_OK, "healthy");
    expect(p.state == PROT_ST_SOFTSTART || p.state == PROT_ST_RUN, "left idle");

    protect_init(&p);
    fill_ok(&s, 14000, 10000);
    expect(protect_eval(&p, &s, 20U) == PROT_OVP, "OVP 14 V");
    expect(p.latch == 1U, "OVP latch");

    protect_init(&p);
    fill_ok(&s, 5000, 120000);
    expect(protect_eval(&p, &s, 20U) == PROT_SCP, "SCP");
    protect_clear(&p);
    expect(p.latch == 1U && p.fault == PROT_SCP, "SCP not clearable");

    protect_init(&p);
    fill_ok(&s, 12000, 10000);
    s.iph_ma[2] = 75000;
    expect(protect_eval(&p, &s, 20U) == PROT_OCP_PH, "phase OCP");

    protect_init(&p);
    fill_ok(&s, 12000, 230000);
    expect(protect_eval(&p, &s, 50U) == PROT_OK, "IOUT OCP not yet");
    expect(protect_eval(&p, &s, 60U) == PROT_OCP_OUT, "IOUT OCP after 100 us");

    protect_init(&p);
    fill_ok(&s, 12000, 10000);
    s.vin_mv = 20000;
    expect(protect_eval(&p, &s, 20U) == PROT_VIN_UV, "VIN UVLO");

    protect_init(&p);
    fill_ok(&s, 12000, 10000);
    s.vin_mv = 70000;
    expect(protect_eval(&p, &s, 20U) == PROT_VIN_OV, "VIN OVLO");

    protect_init(&p);
    protect_note_hw_emb(&p);
    fill_ok(&s, 12000, 10000);
    expect(protect_eval(&p, &s, 20U) == PROT_HW_EMB, "hardware EMB");
}

static void test_control_modes(void)
{
    control_t c;
    sense_eng_t s;
    unsigned n;
    float dmin, dmax;

    fill_ok(&s, 11000, 40000);
    control_init(&c, CTRL_MODE_VOLTAGE);
    control_enable(&c, 1);
    for (n = 0; n < 400; ++n) {
        s.vout_mv += 2;
        if (s.vout_mv > 11950) {
            s.vout_mv = 11950;
        }
        control_step(&c, &s, 50e-6f);
    }
    expect(c.reg == CTRL_REG_CV, "VM stays CV when below Iset");
    expect(c.duty[0] >= DMIN_RUN && c.duty[0] <= DMAX_RUN, "VM duty in range");

    control_init(&c, CTRL_MODE_CURRENT);
    control_enable(&c, 1);
    fill_ok(&s, 11800, 160000);
    for (n = 0; n < 800; ++n) {
        control_step(&c, &s, 20e-6f);
    }
    expect(c.mode == CTRL_MODE_CURRENT, "ACM mode");
    expect(c.iref_tot_a > 1.0f, "voltage loop asked current");
    dmin = dmax = c.duty[0];
    for (n = 0; n < 4; ++n) {
        if (c.duty[n] < dmin) {
            dmin = c.duty[n];
        }
        if (c.duty[n] > dmax) {
            dmax = c.duty[n];
        }
    }
    expect((dmax - dmin) < 0.05f, "ACM phases share duty");

    control_set_cv_cc(&c, 12000, 20000);
    fill_ok(&s, 11500, 25000);
    s.iph_ma[0] = s.iph_ma[1] = s.iph_ma[2] = s.iph_ma[3] = 6250;
    for (n = 0; n < 400; ++n) {
        control_step(&c, &s, 20e-6f);
    }
    expect(c.reg == CTRL_REG_CC, "CC clamp");
    expect(c.iref_tot_a <= 20.5f, "Iset 20 A");

    control_enable(&c, 0);
    control_step(&c, &s, 20e-6f);
    expect(c.duty[0] == 0.0f && c.duty[3] == 0.0f, "disable zeros duty");
}

static void test_adc_host_path(void)
{
    sense_raw_t in, out;

    memset(&in, 0, sizeof(in));
    in.vout = 2978;
    in.iph[1] = 2700;
    adc_acq_init();
    adc_acq_inject_host(&in);
    adc_acq_read(&out);
    expect(out.vout == 2978 && out.iph[1] == 2700, "ADC host inject");
}

int main(void)
{
    test_pins();
    test_csa_ina241();
    test_sense();
    test_hrpwm();
    test_protect();
    test_control_modes();
    test_adc_host_path();

    if (g_fail) {
        fprintf(stderr, "converter tests FAILED\n");
        return 1;
    }
    puts("converter tests OK");
    return 0;
}
