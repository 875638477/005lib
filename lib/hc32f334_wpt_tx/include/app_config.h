#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

/* 评估板按 50 W 电流留量做。WPT_POWER_W 只改合同和保护阈值。 */
#ifndef WPT_POWER_W
#define WPT_POWER_W                 20
#endif

#define CPU_CLK_HZ                  120000000U

#define VIN_MIN_MV                  9000
#define VIN_MAX_MV                  21000
#define VIN_UVLO_MV                 8000
#define VIN_OVLO_MV                 22000

#define VBRG_MIN_MV                 5000
#define VBRG_MAX_MV                 20000
#define VIN_ADC_FS_MV               26400
#define VBRG_ADC_FS_MV              26400

#define FSW_INV_HZ                  127700U
#define FSW_INV_MIN_HZ              110000U
#define FSW_INV_MAX_HZ              205000U
#define FSW_VBRG_HZ                 200000U

#define FRES_LTX_NH                 6300
#define FRES_CS_NF                  400

#define DEADTIME_NS                 70U
#define ANALOG_PING_US              80U
#define COMM_TIMEOUT_MS             1500U

#define CSA_ICOIL_MV_PER_A          100     /* CT 负担后 100 mV/A */
#define CSA_IIN_RSENSE_MOHM         10
#define CSA_IIN_GAIN                20
#define ADC_VREF_V                  3.3f
#define ADC_MAX_LSB                 4095

#define ICOIL_FS_MA                 12000
#define IIN_FS_MA                   8000

#define OTP_DERATE_C                85
#define OTP_TRIP_C                  105

#if WPT_POWER_W == 50
#define P_CONTRACT_MW               50000
#define VBRG_NOM_MV                 18000
#define IIN_OCP_MA                  4500
#define ICOIL_OCP_MA                10000
#define FOD_PLOSS_MW                1800
#elif WPT_POWER_W == 30
#define P_CONTRACT_MW               30000
#define VBRG_NOM_MV                 14000
#define IIN_OCP_MA                  3500
#define ICOIL_OCP_MA                7500
#define FOD_PLOSS_MW                1200
#else
#define P_CONTRACT_MW               20000
#define VBRG_NOM_MV                 11000
#define IIN_OCP_MA                  2800
#define ICOIL_OCP_MA                5500
#define FOD_PLOSS_MW                1000
#endif

#define P_IN_CAP_MW                 ((P_CONTRACT_MW * 110) / 80)

#endif
