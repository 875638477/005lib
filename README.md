# DPS-1200FB 双路电源笼子 / 电源背板参考设计

面向 **HP/HPE Common Slot（CSPS）** 电源模块（以 `DPS-1200FB A` 为代表）的双路热插拔电源笼子与电源背板开源参考设计。

本仓库整理：

- 工作原理与系统架构
- 64Pin 金手指接口定义
- 双路冗余 / 均流背板参考原理图
- PCB 叠层、铜厚与布局建议
- BOM、机械与安全注意事项

> **免责声明**：引脚定义综合社区逆向与公开资料，**非** HP/HPE 官方规格。量产或上电前请用万用表核对具体电源版本。高压与大电流存在触电与起火风险，请具备电气安全能力后再制作。

## 快速导航

| 文档 | 内容 |
| --- | --- |
| [docs/01-architecture.md](docs/01-architecture.md) | 系统架构、笼子与背板分工 |
| [docs/02-working-principle.md](docs/02-working-principle.md) | 使能、热插拔、均流、冗余原理 |
| [docs/03-pinout.md](docs/03-pinout.md) | DPS-1200FB 64Pin 引脚表 |
| [docs/04-reference-design.md](docs/04-reference-design.md) | 双路背板参考设计说明 |
| [docs/05-pcb-guide.md](docs/05-pcb-guide.md) | PCB 设计与工艺规范 |
| [docs/06-safety-test.md](docs/06-safety-test.md) | 安全与上电测试流程 |
| [hardware/schematic/](hardware/schematic/) | 原理图（SVG / 网表 / 分区说明） |
| [hardware/pcb/](hardware/pcb/) | PCB 布局草图与设计约束 |
| [hardware/bom/](hardware/bom/) | 物料清单 |
| [hardware/mechanical/](hardware/mechanical/) | 机械笼子尺寸与装配说明 |

## 核心结论（一句话）

双路笼子背板把两台 `DPS-1200FB` 的 `+12V/GND` 并到公共母排，按槽位独立处理 `PRE/EN`，用不同 `ADR` 挂同一 PMBus，靠电源内部 **Active Droop 均流** 实现 1+1 冗余或并联扩流；背板本身主要做 **连接、分布、使能与监测**，不是第二套 AC-DC。

## 典型规格（单模块）

| 项目 | 典型值 |
| --- | --- |
| 型号 | DPS-1200FB A / HSTNS-PD11 等 |
| 输入 | 100–240 VAC（功率随输入电压变化） |
| 主输出 | +12 V，最高约 100 A（约 1200 W @高电压输入） |
| 待机 | +12VSB，小电流，常开 |
| 接口 | 双面 64Pin 金手指（2.54 mm） |
| 管理 | I2C/PMBus 风格总线（厂商命令集需实测） |

双路并联时：

- **冗余模式**：系统负载按单路额定设计（例如 ≤100 A），一路失效另一路接管
- **扩流模式**：两路同时带载，需严格评估母排、连接器与散热；不要简单把额定电流直接 ×2

## 开源参考与致谢

本设计综合以下公开资料（非官方）：

- [slundell/dps_charger](https://github.com/slundell/dps_charger) — 引脚与 Load Share 说明
- [raplin/DPS-1200FB](https://github.com/raplin/DPS-1200FB) — PIC/I2C 逆向
- ColinTD — Common Slot 使能与 Present 逻辑分析
- [CSPS Common Slot Pinout 整理](https://knightli.com/2026/04/16/csps-common-slot-server-power-supply-pinout/)
- TI SLUA550 — 冗余系统均流与 ORing 应用思路

## 许可

文档与参考设计以学习、实验为目的提供。硬件风险自负；转载请保留出处与免责声明。
