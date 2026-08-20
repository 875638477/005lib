# HC32F334 24–48 V / 4 kW 逆变

四路隔离全桥直流串联升到约 380 V，后级单相 SVPWM 出 220 V / 50 或 60 Hz。

详细方案见 [`docs/HC32F334_24_48V_220V_4kW_四路全桥串联逆变方案.md`](../../docs/HC32F334_24_48V_220V_4kW_四路全桥串联逆变方案.md)。

## 上板要点

- 必须 **LQFP64**（`HC32F334KATI` / `K8TI`）才有 6 组 HRPWM
- 前级超前臂 HRPWM1–4：PC6/PC7、PA8/PA9、PA10/PA11、PB12/PB13
- 前级滞后臂 Timer6：PB4–PB11（须核数据手册复用）
- 后级 HRPWM5/6：PB14/PB15、PC8/PC9
- 硬件封波：PA7 CMP1、PA4 DAC、PC5 EMB

## 主机自测

```
make -C tests test
```

环路、SVPWM、移相和保护是可移植 C。目标工程把 `pwm_front.c` / `pwm_inv.c` / `adc_acq.c` 接到小华 DDL。
