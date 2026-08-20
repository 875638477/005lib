#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "app_config.h"
#include "board_pin.h"
#include "sense_scale.h"
#include "protect.h"
#include "svpwm.h"
#include "pwm_front.h"
#include "pwm_inv.h"
#include "control_dcdc.h"
#include "control_inv.h"
#include "adc_acq.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    expect(k_front_a_pins[0].port == PORT_C && k_front_a_pins[0].pin == 6U, "FB1 AH PC6");
    expect(k_front_a_pins[2].port == PORT_A && k_front_a_pins[2].pin == 8U, "FB2 AH PA8");
    expect(k_front_a_pins[4].pin == 10U, "FB3 AH PA10");
    expect(k_front_a_pins[6].pin == 12U, "FB4 AH PB12");
    expect(k_front_b_pins[0].pin == 4U && k_front_b_pins[7].pin == 11U, "TIM6 PB4-11");
    expect(k_inv_pins[0].pin == 14U && k_inv_pins[2].pin == 8U, "INV HRPWM5/6");
    expect(PHASE_COUNT == 4U, "four series modules");
}

static void test_sense(void)
{
    sense_raw_t raw;
    sense_eng_t eng;
    uint16_t lsb24;
    uint16_t lsb380;

    memset(&raw, 0, sizeof(raw));
    lsb24 = sense_mv_to_vin_lsb(24000);
    lsb380 = sense_mv_to_vbus_lsb(380000);
    expect(abs(sense_vin_lsb_to_mv(lsb24) - 24000) < 40, "24 V VIN reconstruct");
    expect(abs(sense_vbus_lsb_to_mv(lsb380) - 380000) < 200, "380 V BUS reconstruct");

    raw.vin = lsb24;
    raw.vbus = lsb380;
    raw.iser = 2047 + (uint16_t)((10500 * 2047) / ISER_FS_MA);
    raw.vac = 2047 + (uint16_t)((311000 * 2047) / VAC_FS_PK_MV);
    raw.iac = 2047;
    raw.temp = (uint16_t)((45 * ADC_MAX_LSB) / 150);
    raw.vmod[0] = raw.vmod[1] = raw.vmod[2] = raw.vmod[3] =
        (uint16_t)(((int64_t)95000 * ADC_MAX_LSB) / VMOD_ADC_FS_MV);
    raw.iph[0] = raw.iph[1] = raw.iph[2] = raw.iph[3] = 2047;

    sense_raw_to_eng(&raw, &eng);
    expect(abs(eng.vin_mv - 24000) < 50, "eng VIN");
    expect(abs(eng.vbus_mv - 380000) < 250, "eng VBUS");
    expect(abs(eng.vmod_mv[0] - 95000) < 80, "eng Vmod");
    expect(abs(eng.vac_mv - 311000) < 400, "eng VAC peak");
    expect(abs(eng.iser_ma - 10500) < 200, "eng ISER 10.5 A");
}

static void test_svpwm(void)
{
    svpwm_t s;

    svpwm_init(&s, INV_PWM_SVPWM);
    svpwm_update(&s, 0.0f, 0.0f);
    expect(fabsf(s.duty_a - 0.5f) < 1e-5f && fabsf(s.duty_b - 0.5f) < 1e-5f, "SVPWM zero");

    svpwm_update(&s, 0.8f, (float)M_PI / 2.0f);
    expect(fabsf(s.duty_a - 0.9f) < 1e-5f, "SVPWM +peak dA");
    expect(fabsf(s.duty_b - 0.1f) < 1e-5f, "SVPWM +peak dB");
    expect(s.sector == 1U, "SVPWM sector +");
    expect(fabsf(s.duty_a + s.duty_b - 1.0f) < 1e-5f, "SVPWM complementary sum");

    svpwm_update(&s, -0.8f, 3.0f * (float)M_PI / 2.0f);
    expect(fabsf(s.duty_a - 0.1f) < 1e-5f && fabsf(s.duty_b - 0.9f) < 1e-5f, "SVPWM -peak");
    expect(s.sector == 2U, "SVPWM sector -");

    svpwm_set_mode(&s, INV_PWM_UNIPOLAR);
    svpwm_update(&s, 0.6f, 0.0f);
    expect(fabsf(s.duty_a - 0.6f) < 1e-5f && s.duty_b == 0.0f, "unipolar +");
    svpwm_update(&s, -0.6f, 0.0f);
    expect(s.duty_a == 0.0f && fabsf(s.duty_b - 0.6f) < 1e-5f, "unipolar -");

    svpwm_set_mode(&s, INV_PWM_BIPOLAR);
    svpwm_update(&s, 0.4f, 0.0f);
    expect(fabsf(s.duty_a - 0.7f) < 1e-5f && fabsf(s.duty_b - s.duty_a) < 1e-5f, "bipolar");
}

static void test_pwm_front(void)
{
    pwm_front_t h;
    float d24;
    float d48;

    pwm_front_init(&h);
    expect(h.period == 2000U, "60 kHz period @ 120 MHz");
    expect(h.carrier_ticks[0] == 0U && h.carrier_ticks[1] == 500U, "0/90 deg");
    expect(h.carrier_ticks[2] == 1000U && h.carrier_ticks[3] == 1500U, "180/270 deg");
    expect(h.deadtime_ticks == 24U, "200 ns deadtime");

    pwm_front_enable(&h, 1);
    d24 = control_dcdc_feedforward(24000, 380000);
    d48 = control_dcdc_feedforward(48000, 380000);
    expect(d24 > 0.70f && d24 < 0.75f, "Deff @ 24 V ~ 0.72");
    expect(d48 > 0.34f && d48 < 0.38f, "Deff @ 48 V ~ 0.36");

    pwm_front_apply_common(&h, d24);
    expect(h.phase_ticks[0] < h.period / 2U, "24 V more overlap");
    expect(pwm_front_lag_ticks(&h, 1) != pwm_front_lag_ticks(&h, 0), "interleave offset");

    pwm_front_emb_trip(&h);
    expect(h.emb_latched && !h.enabled && h.deff[0] == 0.0f, "EMB zeros front PWM");
    pwm_front_enable(&h, 1);
    expect(!h.enabled, "EMB blocks re-enable");
}

static void test_pwm_inv(void)
{
    pwm_inv_t h;
    svpwm_t s;

    pwm_inv_init(&h);
    expect(h.period == 6000U, "20 kHz period");
    expect(h.deadtime_ticks == 60U, "500 ns inv deadtime");

    svpwm_init(&s, INV_PWM_SVPWM);
    svpwm_update(&s, 0.8f, (float)M_PI / 2.0f);
    pwm_inv_enable(&h, 1);
    pwm_inv_apply(&h, &s);
    expect(pwm_inv_cmp_a(&h) == 5400U, "cmp A 0.9*6000");
    expect(pwm_inv_cmp_b(&h) == 600U, "cmp B 0.1*6000");

    pwm_inv_emb_trip(&h);
    expect(h.emb_latched && pwm_inv_cmp_a(&h) == 3000U, "EMB mid duty");
}

static void fill_ok(sense_eng_t *s)
{
    unsigned i;

    s->vin_mv = 36000;
    s->vbus_mv = 380000;
    s->iser_ma = 10000;
    s->vac_mv = 0;
    s->iac_ma = 0;
    s->temp_c = 45;
    for (i = 0; i < PHASE_COUNT; ++i) {
        s->vmod_mv[i] = 95000;
        s->iph_ma[i] = 25000;
    }
}

static void test_protect(void)
{
    protect_t p;
    sense_eng_t s;

    protect_init(&p);
    fill_ok(&s);
    expect(protect_eval(&p, &s, 1000U) == PROT_OK, "healthy");
    expect(p.state == PROT_ST_INV_SS || p.state == PROT_ST_RUN, "left idle");

    protect_init(&p);
    fill_ok(&s);
    s.vin_mv = 20000;
    expect(protect_eval(&p, &s, 1000U) == PROT_VIN_UV, "VIN UV");

    protect_init(&p);
    fill_ok(&s);
    s.vin_mv = 56000;
    expect(protect_eval(&p, &s, 1000U) == PROT_VIN_OV, "VIN OV");

    protect_init(&p);
    fill_ok(&s);
    s.vbus_mv = 430000;
    expect(protect_eval(&p, &s, 1000U) == PROT_VBUS_OV, "VBUS OV");

    protect_init(&p);
    fill_ok(&s);
    s.vmod_mv[2] = 125000;
    expect(protect_eval(&p, &s, 1000U) == PROT_VMOD_OV, "module OV");

    protect_init(&p);
    fill_ok(&s);
    s.vmod_mv[1] = 60000;
    expect(protect_eval(&p, &s, 10000U) == PROT_OK, "unbal not yet");
    expect(protect_eval(&p, &s, 11000U) == PROT_VMOD_UNBAL, "unbal after 20 ms");

    protect_init(&p);
    fill_ok(&s);
    s.iph_ma[3] = 75000;
    expect(protect_eval(&p, &s, 1000U) == PROT_OCP_PH, "phase OCP");

    protect_init(&p);
    fill_ok(&s);
    s.iser_ma = 25000;
    expect(protect_eval(&p, &s, 1000U) == PROT_OCP_SER, "series OCP");

    protect_init(&p);
    fill_ok(&s);
    s.temp_c = 110;
    expect(protect_eval(&p, &s, 1000U) == PROT_OTP, "OTP");

    protect_init(&p);
    fill_ok(&s);
    protect_note_hw_emb(&p);
    expect(protect_eval(&p, &s, 1000U) == PROT_HW_EMB, "HW EMB");
}

static void test_dcdc(void)
{
    control_dcdc_t c;
    sense_eng_t s;
    unsigned i;

    fill_ok(&s);
    s.vin_mv = 24000;
    s.vbus_mv = 0;
    control_dcdc_init(&c);
    control_dcdc_enable(&c, 1);
    for (i = 0; i < 3; ++i) {
        control_dcdc_step(&c, &s, 1.0f);
    }
    expect(c.vref_ss_mv == VBUS_NOM_MV, "softstart reached 380 V");
    expect(c.deff > 0.65f && c.deff < 0.82f, "24 V Deff after SS");
    expect(fabsf(c.deff_ph[0] - c.deff_ph[3]) < 1e-6f, "IPOS common Deff");

    s.vin_mv = 48000;
    s.vbus_mv = 380000;
    control_dcdc_init(&c);
    control_dcdc_enable(&c, 1);
    c.vref_ss_mv = VBUS_NOM_MV;
    control_dcdc_step(&c, &s, 0.001f);
    expect(c.deff > 0.30f && c.deff < 0.45f, "48 V Deff");

    control_dcdc_enable(&c, 0);
    expect(c.deff == 0.0f, "disable zeros Deff");
}

static void test_inv_freq(void)
{
    control_inv_t c;
    sense_eng_t s;
    float th50;
    float th60;
    unsigned i;

    fill_ok(&s);
    control_inv_init(&c, 50, INV_PWM_SVPWM);
    control_inv_enable(&c, 1);
    control_inv_step(&c, &s, 0.005f);
    th50 = c.theta;
    expect(th50 > 1.5f && th50 < 1.6f, "50 Hz 5 ms -> pi/2");

    control_inv_init(&c, 60, INV_PWM_SVPWM);
    control_inv_enable(&c, 1);
    control_inv_step(&c, &s, 1.0f / 60.0f);
    th60 = c.theta;
    expect(th60 < 1e-3f, "60 Hz one period wraps");
    expect(c.f_out_hz == 60, "freq latched 60");

    control_inv_set_freq(&c, 50);
    expect(c.f_out_hz == 50, "switch back to 50");

    /* Drive one line cycle of samples so RMS updates, then check SVPWM peak. */
    control_inv_init(&c, 50, INV_PWM_SVPWM);
    control_inv_enable(&c, 1);
    for (i = 0; i < 8; ++i) {
        control_inv_step(&c, &s, 0.25f);
    }
    expect(c.m > 0.7f && c.m < 0.95f, "modulation after SS");
    expect(c.svpwm.duty_a >= 0.0f && c.svpwm.duty_a <= 1.0f, "duty in range");

    control_inv_set_mode(&c, INV_PWM_UNIPOLAR);
    s.vac_mv = 0;
    control_inv_step(&c, &s, 1.0f / (float)FSW_INV_HZ);
    expect(c.pwm_mode == INV_PWM_UNIPOLAR, "mode switch");
}

static void test_adc_host(void)
{
    sense_raw_t in;
    sense_raw_t out;

    memset(&in, 0, sizeof(in));
    in.vin = 1500;
    in.vbus = 2800;
    adc_acq_init();
    adc_acq_inject_host(&in);
    adc_acq_read(&out);
    expect(out.vin == 1500 && out.vbus == 2800, "host ADC inject");
}

int main(void)
{
    test_pins();
    test_sense();
    test_svpwm();
    test_pwm_front();
    test_pwm_inv();
    test_protect();
    test_dcdc();
    test_inv_freq();
    test_adc_host();

    if (g_fail) {
        fprintf(stderr, "tests failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
