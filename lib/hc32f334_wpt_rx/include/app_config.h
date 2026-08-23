#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#ifndef WPT_POWER_W
#define WPT_POWER_W                 20
#endif

#define CPU_CLK_HZ                  120000000U

#define FSW_SR_MIN_HZ               110000U
#define FSW_SR_MAX_HZ               205000U
#define FSW_BUCK_HZ                 200000U

#define FRES_LRX_NH                 11000
#define FRES_CS_NF                  230

#define DEADTIME_NS                 60U
#define ASK_BIT_US                  500U
#define ASK_PREAMBLE_ONES           11U
#define COMM_PERIOD_MS              250U

#define ADC_VREF_V                  3.3f
#define ADC_MAX_LSB                 4095
#define VRECT_ADC_FS_MV             33000
#define VOUT_ADC_FS_MV              26400
#define IOUT_FS_MA                  5000

#define OTP_DERATE_C                85
#define OTP_TRIP_C                  105

#if WPT_POWER_W == 50
#define P_CONTRACT_MW               50000
#define VOUT_NOM_MV                 20000
#define IOUT_RATED_MA               2500
#define VRECT_TGT_MV                23000
#define VRECT_OVP_MV                30000
#define VOUT_OVP_MV                 22000
#define IOUT_OCP_MA                 3200
#elif WPT_POWER_W == 30
#define P_CONTRACT_MW               30000
#define VOUT_NOM_MV                 15000
#define IOUT_RATED_MA               2000
#define VRECT_TGT_MV                17500
#define VRECT_OVP_MV                24000
#define VOUT_OVP_MV                 16500
#define IOUT_OCP_MA                 2600
#else
#define P_CONTRACT_MW               20000
#define VOUT_NOM_MV                 12000
#define IOUT_RATED_MA               1670
#define VRECT_TGT_MV                14000
#define VRECT_OVP_MV                20000
#define VOUT_OVP_MV                 13200
#define IOUT_OCP_MA                 2200
#endif

#define CEP_MV_PER_LSB              80
#define VRECT_UV_MV                 4000

#endif
