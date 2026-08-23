# HC32F334 无线充电发射端（20 / 30 / 50 W）

单线圈全桥 PTx。硬件按 50 W 电流留量，软件用 `WPT_POWER_W` 锁合同。

详细评估见 [`docs/HC32F334_无线充电_20_30_50W_方案评估.md`](../../docs/HC32F334_无线充电_20_30_50W_方案评估.md)。

## 上板要点

- 建议 **LQFP64**（`HC32F334KATI` / `K8TI`）
- 逆变：PC6/PC7 + PA8/PA9；VBRG Buck：PA10/PA11
- Icoil CT：PA0；IIN：PA1；VBRG：PA2
- 硬件保护：PA7 CMP1、PA4 DAC、PC5 EMB

## 主机自测

```
make -C tests test
```

环路、ASK、状态机、FOD 是可移植 C。目标工程把 `hrpwm_inv.c` 接到小华 DDL 即可。
