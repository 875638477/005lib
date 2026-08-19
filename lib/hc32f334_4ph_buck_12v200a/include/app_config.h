#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#define PHASE_COUNT                 4U

#define VIN_MIN_MV                  24000
#define VIN_MAX_MV                  60000
#define VIN_UVLO_MV                 22000
#define VIN_OVLO_MV                 65000

#define VOUT_NOM_MV                 12000
#define VOUT_ADC_FS_MV              16500
#define VIN_ADC_FS_MV               79700

#define IOUT_RATED_MA               200000
#define IPH_RATED_MA                50000
#define IPH_FS_MA                   80000
#define IOUT_FS_MA                  250000

#define FSW_HZ                      200000U
#define CPU_CLK_HZ                  120000000U
#define HRPWM_PERIOD_TICKS          (CPU_CLK_HZ / FSW_HZ)

#define DEADTIME_NS_SI              80U
#define DEADTIME_NS_GAN             30U
#define DEADTIME_NS                 DEADTIME_NS_SI

#define PHASE_SHIFT_DEG             {0, 90, 180, 270}

#define CTRL_VM_HZ                  20000U
#define CTRL_CM_HZ                  50000U

#define SOFTSTART_V_PER_S           50
#define DMAX_SOFTSTART              0.55f
#define DMAX_RUN                    0.72f
#define DMIN_RUN                    0.02f

#define OCP_IPH_MA                  70000
#define OCP_IOUT_MA                 220000
#define OCP_IOUT_HOLD_US            100U
#define SCP_VOUT_MV                 6000
#define SCP_IOUT_MA                 80000
#define OVP_SOFT_MV                 13800
#define OVP_HW_MV                   14500
#define OTP_C                       105

#define CSA_GAIN                    20.0f
#define CSA_RSENSE_OHM              0.00050f
#define CSA_VREF_V                  1.65f
#define ADC_VREF_V                  3.3f
#define ADC_MAX_LSB                 4095

#define IOUT_USE_HALL               1
#define IOUT_HALL_FS_MA             250000
#define IOUT_HALL_V_FS              3.3f

#endif
