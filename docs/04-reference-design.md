# 04 — 双路背板参考设计

## 1. 设计目标

| 项目 | Rev A（本仓库默认） |
| --- | --- |
| 槽位数 | 2 × DPS-1200FB / 兼容 Common Slot |
| 输出 | 公共 +12V / GND 母排 |
| 均流 | 依赖模块 Active Droop |
| ORing | 无（直并） |
| 使能 | 每槽独立拨码/开关，可并联为总使能 |
| 监测 | 每槽 PSOK LED + ALARM LED；PMBus 焊盘 |
| 待机 | 每槽 PRE 上拉；SB 可选二极管汇流 |
| 输出接口 | 4× M8 铜柱（2×12V + 2×GND）或铜排 |

## 2. 功能框图

见 `hardware/schematic/dual-cage-block.svg` 与下方文字版：

```text
        PSU-A 插座                         PSU-B 插座
     ┌───────────────┐                  ┌───────────────┐
     │ +12V ─┐       │                  │ +12V ─┐       │
     │ GND  ─┼─►母排 │                  │ GND  ─┼─►母排 │
     │ SB    │       │                  │ SB    │       │
     │ PRE/EN/PSOK   │                  │ PRE/EN/PSOK   │
     │ SCL/SDA/ADR   │                  │ SCL/SDA/ADR   │
     └───────────────┘                  └───────────────┘
              │                                  │
              └────────────┬─────────────────────┘
                           ▼
                 VBUS_12V / PGND 输出区
                 SBS_BUS → 3V3 LDO（可选）
                 PMBus 头 / LED / 开关
```

## 3. 电路分区说明

详细网表：`hardware/schematic/dual-cage-netlist.md`  
分区原理图：`hardware/schematic/sheets/*.md`  
总览 SVG：`hardware/schematic/dual-cage-schematic.svg`

### 3.1 功率汇流

- 将槽 A/B 所有 `+12V` 针并到 `VBUS_12V`
- 将所有 `GND` 针并到 `PGND`
- 在靠近每个插座处放置若干低 ESL 陶瓷电容（如 10µF/25V × 多个）+ 大电解/固态（如 1000µF/16V 级，按空间选）
- 输出铜柱周围开窗上锡或加铜排降低温升

### 3.2 Present / Enable

每槽：

```text
SB_x ── Rpre(22k) ── PRE_x ──(到模块 Pin36)
                    │
                   测试点

3V3/内部上拉(模块内) ── EN_x# ── SW_x ── SGND
                              └── 也可到 MCU 开漏
```

可选：`EN_A#` 与 `EN_B#` 经二极管或 divers 接到 `EN_GLOBAL#`。

### 3.3 状态指示

```text
PSOK_x ── R ── LED ── SB_x/3V3   （按实测电平选择限流与极性）
ALARM_x ── R ── LED
```

PSOK 在不同版本可能是推挽或开漏倾向，原理图用串联电阻 + LED 到合适电源，并预留跳线。

### 3.4 PMBus

```text
SCL_A ──┬── SCL_B ──┬── Rpu ── 3V3_LOGIC
SDA_A ──┬── SDA_B ──┬── Rpu ── 3V3_LOGIC
SGND 公共

ADR：槽A 保持默认；槽B 将 ADR 相关脚经 0Ω 下拉到 SGND（按实测表选择）
```

上拉电阻典型 2.2k–10k，先用 4.7k，用示波器看边沿再调。

### 3.5 12VSB 汇流（可选）

```text
SB_A ── Schottky ──┬── SBS_BUS ──► LDO 3V3 / 风扇逻辑 / LED 公共阳极
SB_B ── Schottky ──┘
```

防止一槽无 AC 时另一槽 SB 反灌。

### 3.6 Rev B 扩展：ORing

每槽功率路径：

```text
V12x ── MOSFET( Controllable ORing ) ── VBUS_12V
              ▲
         TPS2412/2413 等控制器
```

详见 `hardware/schematic/sheets/05-oring-optional.md`。

## 4. 关键器件选型（Rev A）

| 位号思路 | 推荐 | 备注 |
| --- | --- | --- |
| J1/J2 | 64Pin 2.54 边缘插座 | 确认键位/高度 |
| Rpre | 22k 0805 | PRE→SB |
| SW | 拨码/船型开关 2A 即可 | 只切信号 |
| LED | 0805 | 限流 1k–4.7k |
| Cdec | 10µF/25V X5R ×8/槽 | 近插座 |
| Cbulk | 470–2200µF/16V | 母排
| 输出 | M8 铜柱 | 双 12V + 双 GND |
| TVS | SMAJ15A 等 | 母排可选 |
| LDO | 12VSB→3V3（AMS1117-3.3 等仅小电流） | MCU/上拉 |

完整 BOM：`hardware/bom/bom-revA.csv`

## 5. 机械配合

- 两槽中心距按笼子导轨确定（需对照你采购的笼子或自制钣金）
- 插座伸出量保证金手指全插入后外壳卡扣到位
- 背板固定孔与笼子绝缘柱对齐，避免短路
- 详见 `hardware/mechanical/cage-notes.md`

## 6. 非目标（本 Rev 不做）

- 不改 PSU 内部调压 / 抬 OVP
- 不把两路输出串联做 24V（需浮地改装，超出本背板范围）
- 不提供 AC 输入配电板（AC 仍走模块自身 C14/C13）
