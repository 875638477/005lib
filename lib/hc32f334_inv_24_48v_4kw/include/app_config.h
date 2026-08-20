#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#define PHASE_COUNT                 4U

#define VIN_MIN_MV                  22000
#define VIN_NOM_LO_MV               24000
#define VIN_NOM_HI_MV               48000
#define VIN_MAX_MV                  52000
#define VIN_UVLO_MV                 22000
#define VIN_OVLO_MV                 54000

#define VBUS_NOM_MV                 380000
#define VBUS_UV_MV                  320000
#define VBUS_OV_SOFT_MV             420000
#define VBUS_OV_HW_MV               430000
#define VMOD_NOM_MV                 95000
#define VMOD_OV_MV                  120000
#define VMOD_UNBAL_PCT              10
#define VMOD_UNBAL_MS               20

#define VAC_RMS_NOM_MV              220000
#define VAC_RMS_OV_MV               242000
#define VAC_PEAK_NOM_MV             311126

#define IIN_RATED_MA                185000
#define IPH_RATED_MA                50000
#define IPH_OCP_MA                  70000
#define IPH_HW_MA                   80000
#define ISER_RATED_MA               10500
#define ISER_OCP_MA                 20000
#define ISER_HW_MA                  35000
#define IAC_RMS_OL_MA               28000
#define IAC_SURGE_MA                45000
#define IAC_HW_MA                   70000

#define P_RATED_W                   4000
#define XFMR_RATIO                  5.5f
#define DEFF_MIN                    0.05f
#define DEFF_MAX                    0.82f
#define DEFF_SOFTSTART_MAX          0.55f

#define FSW_DCDC_HZ                 60000U
#define FSW_INV_HZ                  20000U
#define F_OUT_50_HZ                 50U
#define F_OUT_60_HZ                 60U
#define F_OUT_DEFAULT_HZ            F_OUT_50_HZ

#define CPU_CLK_HZ                  120000000U
#define DCDC_PERIOD_TICKS           (CPU_CLK_HZ / FSW_DCDC_HZ)
#define INV_PERIOD_TICKS            (CPU_CLK_HZ / FSW_INV_HZ)

#define DCDC_DEADTIME_NS            200U
#define INV_DEADTIME_NS             500U

#define CTRL_DCDC_HZ                4000U
#define CTRL_INV_HZ                 FSW_INV_HZ
#define SOFTSTART_VBUS_V_PER_S      200
#define SOFTSTART_M_PER_S           0.40f

#define M_MIN                       0.02f
#define M_MAX                       0.95f

#define OTP_C                       105
#define FAN_ON_C                    45

#define ADC_VREF_V                  3.3f
#define ADC_MAX_LSB                 4095
#define VIN_ADC_FS_MV               66000
#define VBUS_ADC_FS_MV              550000
#define VMOD_ADC_FS_MV              165000
#define ISER_FS_MA                  40000
#define IPH_FS_MA                   80000
#define IAC_FS_MA                   80000
#define VAC_FS_PK_MV                400000
#define CSA_GAIN                    20.0f
#define CSA_RSENSE_OHM              0.001f
#define CSA_VREF_V                  1.65f

typedef enum {
    INV_PWM_SVPWM = 0,
    INV_PWM_UNIPOLAR,
    INV_PWM_BIPOLAR
} inv_pwm_mode_t;

#endif
