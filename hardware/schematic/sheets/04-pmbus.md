# Sheet 04 — PMBus / 地址 / 待机电源

## 总线

```text
J1.31 SCL ----+---- J2.31 ----+---- R3 4.7k ---- 3V3
                              +---- J3.SCL
                              +---- ESD/TVS optional to SGND

J1.32 SDA ----+---- J2.32 ----+---- R4 4.7k ---- 3V3
                              +---- J3.SDA

J1.30 / J2.30 SGND ---- J3.GND ---- LDO GND ---- PGND(单点)
```

## 地址绑线（示例，务必实测）

目标：槽 A 与槽 B 地址不同。

```text
槽 A: ADR_A0/1/2 默认悬空或按模块默认上拉（装 0R DNP 位）
槽 B: 将 ADR_B0/1/2 中需要的脚经 0R 拉到 SGND
```

社区常见：拉低 27/28/29 可使 EEPROM/MCU 地址下移。以扫描结果为准。

## 12VSB 汇流与 3V3

```text
SB_A -- D1 Schottky --+-- SBS_BUS --+-- Cin -- U1 LDO -- 3V3 -- Cout
SB_B -- D2 Schottky --+             |
                                    +-- 给 LED 公共 / 上拉 / MCU
```

LDO 仅供逻辑，注意功耗与散热。若只用跳线使能、无需 MCU，U1 可 DNP，上拉改挂到 SBS_BUS（确认电平兼容）。
