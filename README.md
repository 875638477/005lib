# 24V → 220V / 4000W 逆变器（HPM6284）

离网纯正弦逆变器的主控与功率方案：24V 电池输入，220V / 50Hz / 4000W 输出，主控 **HPM6284IPA（eLQFP144）**。

## 文档

完整设计（拓扑、电参数、引脚、采样、保护）见：

[docs/design/24v_220v_4kw_inverter_hpm6284.md](docs/design/24v_220v_4kw_inverter_hpm6284.md)

## 固件骨架

| 文件 | 作用 |
| --- | --- |
| `firmware/board/hpm6284_inverter_pinmux.h` | PWM / ADC / GPIO 引脚 |
| `firmware/board/sensing_config.h` | 分流、分压、ADC 换算 |
| `firmware/board/protection_config.h` | 保护阈值与故障位 |
| `firmware/board/pinmux.c` | IOC 初始化（需 HPM SDK） |

## 拓扑一句话

**四相交错全桥**（24V → 380V 隔离母线，90° 交错）+ 单极性 SPWM 全桥（380V → 220V AC）。  
每相约 46A / 1.1kW；PWM0 驱相 1–2，PWM3 驱相 3–4，PWM1 驱逆变。
