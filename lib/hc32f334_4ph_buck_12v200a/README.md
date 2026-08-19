# HC32F334 四相交错 12 V / 200 A

24–60 V 输入，同步 Buck，90° 交错。高边电流检测默认 **INA241A2**。

详细方案见 [`docs/HC32F334_24V60V_12V200A_四相交错方案.md`](../../docs/HC32F334_24V60V_12V200A_四相交错方案.md)。

## 上板要点

- 必须 **LQFP64**（`HC32F334KATI` / `K8TI`）
- HRPWM：PC6/PC7、PA8/PA9、PA10/PA11、PB12/PB13
- IPH：PA0–PA3 → INA241A2
- 硬件保护：PA7 CMP1（OCP 线或）、PA4 DAC 门槛、PC5 EMB

## 主机自测

```
make -C tests test
```

环路与保护是可移植 C。目标工程把 `hrpwm_4ph.c` / `adc_acq.c` 接到小华 DDL 即可。
