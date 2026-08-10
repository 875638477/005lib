# 双路电源背板网表（Rev A）

逻辑网表，便于导入 KiCad / Altium。连接器引脚按 Common Slot 社区定义。

## 连接器 J1 — SLOT A

| Pin | Net |
| --- | --- |
| 1–13, 52–64 | V12A |
| 14–26, 39–51 | PGND |
| 27 | ADR_A0 |
| 28 | ADR_A1 |
| 29 | ADR_A2 |
| 30 | SGND |
| 31 | SCL |
| 32 | SDA |
| 33 | EN_A# |
| 34 | ISHARE_A |
| 35 | PSOK_A |
| 36 | PRE_A |
| 37 | SB_A |
| 38 | ALARM_A |

## 连接器 J2 — SLOT B

| Pin | Net |
| --- | --- |
| 1–13, 52–64 | V12B |
| 14–26, 39–51 | PGND |
| 27 | ADR_B0 |
| 28 | ADR_B1 |
| 29 | ADR_B2 |
| 30 | SGND |
| 31 | SCL |
| 32 | SDA |
| 33 | EN_B# |
| 34 | ISHARE_B |
| 35 | PSOK_B |
| 36 | PRE_B |
| 37 | SB_B |
| 38 | ALARM_B |

## 功率直并（Rev A）

```text
V12A ──► VBUS_12V
V12B ──► VBUS_12V
PGND 公共
SGND ──(近板中心单点/多点阵列)── PGND
```

## 无源与开关

```text
R1  22k  SB_A  → PRE_A
R2  22k  SB_B  → PRE_B
SW1      EN_A# → SGND
SW2      EN_B# → SGND
R3  4.7k 3V3   → SCL
R4  4.7k 3V3   → SDA
D1  SS34 SB_A  → SBS_BUS
D2  SS34 SB_B  → SBS_BUS
U1  LDO  SBS_BUS → 3V3 （输入输出电容按数据手册）
R5  0R/DNP  ADR_A* 配置
R6.. 0R    ADR_B* 下拉到 SGND（使地址不同于 A）
LED1+R     PSOK_A
LED2+R     PSOK_B
LED3+R     ALARM_A
LED4+R     ALARM_B
C*         VBUS_12V↔PGND 去耦与储能
TP*        各关键网络测试点
J3         PMBus 针座：SCL, SDA, 3V3, SGND
JOUT       M8 铜柱：VBUS_12V ×2, PGND ×2
```

## 可选跳线

| 位号 | 功能 |
| --- | --- |
| JP1 | 短接 EN_A# 与 EN_B# 实现总开关 |
| JP2 | 连接 ISHARE_A–ISHARE_B（默认断开） |
| JP3 | 3V3 上拉来源选择（SBS LDO / 外部） |
