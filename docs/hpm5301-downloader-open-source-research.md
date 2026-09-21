# 基于 HPM5301 的下载器开源软硬件调研记录

> 调研日期：2026-09-15
> 调研对象：MicroLink 以及可用于实现同类下载器的 HPM5301 开源项目
> 记录范围：硬件资料、设备固件、上位机、构建方式、许可证、功能差异和建议实现路线

## 1. 调研结论

目前能找到四条主要技术路线：

1. **MicroLink** 最接近“在线调试器 + USB 串口 + U 盘拖拽 + 离线下载器 + 脚本化升级工具”的完整产品形态。
2. **HSLink Pro** 的软硬件、上位机和生产资料最完整，许可证也最明确，适合作为自行开发产品的基础。
3. **YBLINK** 在一个仓库内提供纯 Rust 固件、Altium 原理图、PCB、Gerber 和预编译固件，适合学习最小完整实现。
4. **CherryDAP HPM5301EVKLite** 可以直接在官方开发板上验证 CMSIS-DAP、SWD/JTAG 和 USB 转串口，适合最早期的软件验证。

需要特别区分“源码公开”和“开源”：

- MicroLink 仓库公开了大量设备端源码，但仓库根目录没有许可证。
- MicroLink 硬件目录目前只有主控原理图 PDF 和 Altium 预览缓存，没有可编辑原理图、PCB、Gerber、BOM。
- 因此，截至本次调研日期，MicroLink **不能视为具备完整、明确授权的开源软硬件参考设计**。
- HSLink Pro 和 YBLINK 的授权与生产资料更适合直接复用。

## 2. HPM5301 基础信息

### 2.1 官方公开规格

HPM5301 属于先楫半导体 HPM5300 系列，主要特点包括：

- 单核 32 位 RISC-V MCU；
- HPM5301IEG1 为 6 mm × 6 mm、0.4 mm 间距的 QFN48_EP 封装；
- 288 KB SRAM；
- 1 MB 片内 Flash；
- 一个内置 PHY 的 USB High-Speed OTG 控制器；
- USB High-Speed 总线速率为 480 Mbps；
- 多路 UART、SPI、I²C 等外设。

### 2.2 主频信息的差异

不同项目和官方页面中的描述并不完全一致：

- HPM5300 **系列**官方页面描述为“主频高达 480 MHz”；
- 先楫发布 HPM5301 及 HPM5301EVKLite 时的产品资料标注 HPM5301 为 **360 MHz**；
- MicroLink README 自述其 HPM5301 工作于 **480 MHz**。

因此，“HPM5301 主频 480 MHz”不能仅根据 MicroLink README 当作芯片额定规格。复刻时应核对：

1. 实际采购料号和芯片版本；
2. 最新数据手册和勘误表；
3. 固件 PLL、DCDC 和 Flash 等待周期配置；
4. 480 MHz 是否属于超频运行。

USB 的“480 Mbps”表示 USB High-Speed 总线速率，不等于 MCU 主频。

官方资料入口：

- [HPM5300 系列产品页](https://www.hpmicro.com/product-center/microcontroller/hpm5300)
- [HPM5301EVKLite 板级文档](https://hpm-sdk.readthedocs.io/en/latest/boards/hpm5301evklite/README_en.html)

## 3. MicroLink 调研

### 3.1 项目入口

- 代码仓库：[Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)
- 使用文档：[MicroBoot 文档中的 MicroLink/MKLink 页面](https://microboot.readthedocs.io/zh-cn/latest/tools/microlink/microlink/)
- 相关升级框架：[Aladdin-Wang/MicroBoot](https://github.com/Aladdin-Wang/MicroBoot)

### 3.2 已公开功能

MicroLink README 声明支持：

- CMSIS-DAP 在线下载和调试；
- SWD 和 JTAG；
- OpenOCD 方式调试 Arm、RISC-V 等目标芯片；
- 最高 10 MHz 的 SWD/JTAG 时钟；
- USB 转串口，标称最高 10 Mbaud；
- RTT 数据转发到虚拟串口；
- Cortex-M 的 U 盘拖拽下载；
- BIN、HEX 文件烧录；
- 通过 PikaPython 脚本触发离线下载；
- 加载从 CMSIS-Pack/Keil FLM 转换得到的下载算法；
- Ymodem 文件发送；
- 下载器自身固件升级；
- 3.3 V/5 V 目标供电、防倒灌和过流保护。

上述性能数据来自项目自述，本次调研未进行独立硬件测试。

### 3.3 软件架构

从公开源码目录可以整理出以下模块：

| 功能层 | 主要目录/文件 | 作用 |
| --- | --- | --- |
| CMSIS-DAP | `MicroLink/external/DAP` | DAP 命令、SWD、JTAG 等协议处理 |
| USB 协议栈 | `microlink_app/.../middleware/cherryusb` | USB Device 和复合设备传输 |
| DAP 主任务 | `microlink_app/src/dap_main.*` | DAP 数据收发和调度 |
| USB U 盘 | `microlink_app/src/USB2MSC` | Mass Storage Class 设备 |
| USB 串口 | `microlink_app/src/USB2UART` | CDC/串口桥 |
| Python 控制通道 | `microlink_app/src/USB2Python` | 通过 USB 虚拟串口输入脚本或命令 |
| Python 引擎 | `MicroLink/external/pikapython` | 设备内脚本解释与扩展 API |
| 虚拟文件系统 | `microlink_app/vfs` | 拖拽文件、HEX 解析、Flash 下载调度 |
| 拖拽下载 | `microlink_app/vfs/drag-n-drop` | 文件流、Intel HEX、VFS 管理 |
| Flash 算法执行 | `microlink_app/vfs/target` | Flash blob、算法解码、编程接口 |
| SWD 主机 | `microlink_app/src/swd_host` | 访问目标 MCU 的调试端口和内存 |
| Ymodem | `microlink_app/src/ymodem_send` | 向目标设备发送升级文件 |
| 板级支持 | `MicroLink/hpm5301evklite` | HPM5301 板级配置和 Pinmux |
| HPM SDK | `hpm_sdk_localized_for_hpm5301evklite` | 被裁剪并放入仓库的 HPM SDK |
| 升级打包 | `ota_packager` | Windows OTA/RBL 打包工具文件 |

其整体数据流可以概括为：

```text
PC / 按键
    │
    ├── CMSIS-DAP Bulk/HID ──> DAP 命令 ──> SWD/JTAG ──> 目标 MCU
    ├── USB CDC ─────────────> UART/RTT/Python 命令
    └── USB MSC ─────────────> VFS/文件解析
                                  │
                                  ├── HEX/BIN
                                  ├── FLM.o 下载算法
                                  └── PikaPython 离线流程
                                           │
                                           └── SWD Host ──> 目标 Flash
```

### 3.4 离线下载实现思路

MicroLink 的离线下载不是为每种目标 MCU 固化一套烧录代码，而是组合以下资源：

1. 待烧录的 BIN 或 HEX 文件；
2. 从厂商 CMSIS-Pack 中取得的 FLM 下载算法；
3. 转换后的 `*.FLM.o` 算法文件；
4. `flm_config.py` 或 `offline_download.py` 流程脚本；
5. SWD Host 负责访问目标 MCU 内存和寄存器；
6. 下载器把算法和数据写入目标 RAM，再调用算法中的擦除、编程、校验入口。

README 中给出的典型接口包括：

```python
ReadFlm.load("STM32/STM32F10x_512.FLM.o", 0x08000000, 0x20000000)
load.bin("boot.bin", 0x08000000)
ym.send("update.bin")
```

该设计的主要优点是：

- 不必在下载器固件中静态集成所有目标芯片算法；
- 可由脚本组合擦除、烧录、验证、串口升级和产测流程；
- 适合离线售后和小批量生产。

主要风险包括：

- FLM 文件及其转换产物的再分发授权取决于芯片厂商或 CMSIS-Pack 的许可证；
- 必须隔离错误算法，避免访问下载器自身内存或造成设备死机；
- 目标 RAM 地址、栈、算法工作区和数据缓冲区需要避免重叠；
- 需要定义断电恢复、失败重试、校验及日志策略。

### 3.5 硬件资料开放程度

MicroLink 根目录存在 `原理图 V2.0`，但实际内容只有：

- `主控.pdf`；
- `5301.SchDocPreview`；
- `HMP5301.SchDocPreview`；
- `XL8812x3.SchDocPreview`；
- 若干 `PcbDocPreview`。

未发现：

- 原始 `.SchDoc`；
- 原始 `.PcbDoc`；
- KiCad 工程；
- Gerber；
- BOM；
- 坐标文件；
- 完整生产说明。

Altium 的 `*Preview` 文件不能代替可编辑工程。当前资料最多可用于阅读部分电路，不能保证无损还原和直接投产。

### 3.6 许可证结论

GitHub API 未识别到 MicroLink 根许可证，仓库树中也未发现适用于整个项目的根 `LICENSE` 或 `COPYING`。目录中出现的许可证仅属于 HPM SDK、perf_counter 等第三方组件。

这意味着：

- 可以阅读公开代码；
- 不能默认获得复制、修改、发布或商用的权利；
- 第三方组件的开源许可证不会自动覆盖 MicroLink 自有代码；
- 在作者补充许可证或书面授权前，不建议直接把 MicroLink 自有代码放入要发布或销售的产品。

### 3.7 迷你示波器与逻辑分析仪

公开仓库和后续 MKLink 文档中的“示波器/分析仪”不是 ADC 或 GPIO 仪器：

- 迷你示波器：SWD 周期读取目标 RAM 变量，封装 VOFA+ JustFloat，经 USB CDC 画曲线；
- SystemView：SWD 读取目标 RTT 通道 1 的 RTOS 事件，转发到上位机时间轴；
- README 中的逻辑分析仪图片是外部仪器测量 SWD/UART，不是设备内置 GPIO 采样。
- 其中 `10M_TTL.jpg` 的窗口是 Kingst LA2016 / KingstVIS。图里的 `I/O电平标准` 是外部探头阈值，不是 MicroLink 用 IO 做采集。
- 若自己做最基础的 GPIO 逻辑分析仪：HPM5301 可以。建议 8 路、1 到 10 MSa/s、32 到 128 Ki 点、先采后传、SUMP/PulseView。做不到 LA2016 那种 200 MHz 和可调阈值。优先在 EVKLite 上用 GPIO0 + 定时器/DMA 验证，再考虑塞进下载器。

详细实现、命令、帧格式和源码对应关系见
[scope-and-logic-analyzer.md](../001DIY/009ST_LINK_XDS110/03Daplink/scope-and-logic-analyzer.md)。

自行重做硬件、外挂 SPI Flash、双串口、OLED 和 8 路逻辑分析仪的 IO 分配见
[hardware-io-map.md](../001DIY/009ST_LINK_XDS110/03Daplink/hardware-io-map.md)。

## 4. HSLink Pro 调研

### 4.1 项目组成

- 设备固件：[cherry-embedded/CherryDAP](https://github.com/cherry-embedded/CherryDAP)
- HSLink Pro 固件目录：[projects/HSLink-Pro](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/HSLink-Pro)
- 硬件工程：[cherry-embedded/HSLink-hardware](https://github.com/cherry-embedded/HSLink-hardware)
- 上位机：[HSLink/HSLinkNexus](https://github.com/HSLink/HSLinkNexus)
- 制作指南：[HSLinkCompact v1.3.1 ProductionGuide](https://github.com/cherry-embedded/HSLink-hardware/blob/main/HSLinkCompact_v1.3.1/ProductionGuide.md)

旧链接 `HSLink/HSLink_Hardware` 会跳转到当前的
`cherry-embedded/HSLink-hardware`。

### 4.2 许可证

| 部分 | 许可证 |
| --- | --- |
| CherryDAP/设备固件 | Apache-2.0 |
| HSLink-hardware | Apache-2.0 |
| HSLinkNexus 上位机 | Apache-2.0 |
| HPM SDK | BSD-3-Clause |

复用时仍需保留许可证、版权声明和 NOTICE 要求，并逐一检查仓库中的第三方组件。

### 4.3 硬件资料

硬件仓库提供：

- KiCad 原理图和 PCB；
- 分层原理图；
- 原理图 PDF；
- BOM、位号和坐标文件；
- 生产归档；
- 制作指南；
- 多个历史硬件版本。

当前仓库中可见的主要版本包括：

- HSLinkPro v1.0；
- HSLinkPro v1.2.0；
- HSLinkPro v1.2.1；
- HSLinkCompact v1.3.1；
- HSLinkIso v1.2.2。

HSLinkCompact v1.3.1 需要 KiCad 9 或以上版本打开。

### 4.4 功能

项目文档声明支持：

- CMSIS-DAP 2.1；
- SWD 和 JTAG；
- GPIO 和 SPI 两种时序输出方式；
- 标称最高 80 MHz 调试时钟；
- 目标电平转换；
- 可控目标板供电，标称最高 1 A；
- USB 转串口；
- DTR/RTS，可用于 ESP32 等设备自动下载；
- 多种硬复位和 Arm 软复位方式；
- 设置持久化；
- UF2 拖拽升级调试器固件；
- 上位机配置与升级。

HSLink Pro 是成熟的高速在线调试器，但当前公开功能不包含 MicroLink 式的
PikaPython + FLM 离线量产下载。

## 5. YBLINK 调研

### 5.1 项目入口

- 仓库：[ref42/YBLINK](https://github.com/ref42/YBLINK)
- 中文说明：[README.zh.md](https://github.com/ref42/YBLINK/blob/master/README.zh.md)

### 5.2 开放内容

仓库同时包含：

- 纯 Rust HPM5301 固件；
- HPM5301 PAC 和 HAL；
- CMSIS-DAP v2 实现；
- SWD/JTAG FGPIO 时序；
- USB High-Speed DAP 接口；
- CDC ACM USB 转串口；
- Altium `.SchDoc`、`.PcbDoc`；
- 原理图 PDF；
- Gerber 压缩包；
- 预编译固件。

仓库提供 MIT 和 Apache-2.0 两份许可证文件。实际复用时应按仓库声明选择适用许可证并保留相关文本。

### 5.3 已公开引脚

| 功能 | HPM5301 引脚 |
| --- | --- |
| SWCLK/TCK | PA27 |
| SWDIO/TMS | PA28 |
| JTAG TDI | PA29 |
| JTAG TDO | PA26 |
| 目标 nRESET | PB10 |
| UART0 TXD | PA00 |
| UART0 RXD | PA01 |

目标板必须自行供电并与 YBLINK 共地。

### 5.4 构建与烧录

```bash
rustup target add riscv32imafc-unknown-none-elf
cargo build -p yblink --release
probe-rs download --chip HPM5301 --protocol jtag \
  target/riscv32imafc-unknown-none-elf/release/yblink
```

仓库还提供 `firmware/yblink` 预编译文件。

### 5.5 适用范围

YBLINK 适合：

- 学习 HPM5301 USB HS 和 CMSIS-DAP 的最小实现；
- 使用 Rust 开发新的调试器；
- 快速制作固定 3.3 V、目标板独立供电的调试探针。

它当前不等价于 MicroLink，缺少：

- U 盘拖拽下载；
- 离线文件存储；
- FLM 动态加载；
- Python 脚本；
- 目标供电和电平转换。

## 6. CherryDAP HPM5301EVKLite

### 6.1 项目入口

- 固件目录：[CherryDAP/projects/hpm5301evklite](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/hpm5301evklite)
- 使用文档：[HPM5301EVKLite | CherryDAP](https://cherrydap.cherry-embedded.org/projects/HPM5301EVKLite)

### 6.2 功能和用途

该工程支持：

- USB High-Speed；
- CMSIS-DAP；
- SWD；
- JTAG；
- DMA USB 转串口；
- 直接运行在 HPM5301EVKLite 开发板上。

这是验证设备固件最省硬件风险的路线。先在开发板上打通：

1. USB 枚举；
2. CMSIS-DAP；
3. SWD/JTAG；
4. USB 转串口；
5. 目标芯片下载；

再设计定制 PCB，可以减少同时排查硬件和固件问题的工作量。

构建依赖 HPM SDK 和 CMake；项目文档要求 HPM SDK 不低于其指定版本。使用时应以仓库当前 README 和 CI 配置为准。

## 7. 其他相关项目

### 7.1 miniHslink

- 仓库：[KotoriProject/miniHslink](https://github.com/KotoriProject/miniHslink)
- 内容：KiCad 原理图和 PCB。
- 状态：GitHub 未识别到许可证。

它可以作为布板参考，但与 MicroLink 一样，在没有明确许可证时不应默认具有生产和再发布授权。

### 7.2 HPM 开源硬件索引

- 仓库：[RCSN/hpm_open_hardware](https://github.com/RCSN/hpm_open_hardware)

该仓库汇总了基于 HPMicro MCU 的社区硬件，包括 HSLink、miniHslink、HPM5301 开发板和其他调试器，可用于继续寻找机械尺寸、电源、隔离和接口参考设计。

### 7.3 MicroBoot

- 仓库：[Aladdin-Wang/MicroBoot](https://github.com/Aladdin-Wang/MicroBoot)
- 许可证：Apache-2.0

MicroBoot 是目标设备侧的升级框架，包含 Ymodem 等升级相关文档。它不是 HPM5301 调试器固件，但可以与下载器的 Ymodem 发送功能配合。

## 8. 方案对比

| 项目 | 主控/平台 | SWD | JTAG | USB 串口 | U盘/MSC | 离线下载 | 可编辑硬件 | 生产资料 | 根许可证 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| MicroLink | HPM5301 | 是 | 是 | 是 | 是 | 是，PikaPython + FLM | 否，仅 PDF/预览 | 否 | 无 |
| HSLink Pro | HPM5301 | 是 | 是 | 是 | UF2 升级盘 | 否 | 是，KiCad | 是 | Apache-2.0 |
| YBLINK | HPM5301 | 是 | 是 | 是 | 否 | 否 | 是，Altium | Gerber | MIT/Apache-2.0 |
| CherryDAP EVKLite | HPM5301EVKLite | 是 | 是 | 是 | 否 | 否 | 使用开发板 | 不适用 | Apache-2.0 |
| miniHslink | HPM5301 | 取决于固件 | 取决于固件 | 取决于固件 | 否 | 否 | 是，KiCad | 未确认 | 无 |

## 9. Keil、IAR 和其他开发工具兼容性

### 9.1 兼容性判断原则

MicroLink、HSLink Pro、YBLINK 和 CherryDAP HPM5301EVKLite 都以
CMSIS-DAP 作为主要在线调试协议。因此，能否接入开发工具主要由以下条件决定：

1. 固件是否正确实现 CMSIS-DAP v1/v2、SWD 或 JTAG；
2. 开发工具是否有 CMSIS-DAP 驱动，或者是否能连接 OpenOCD/pyOCD GDB Server；
3. 开发工具是否支持被调试的目标 CPU 架构；
4. 是否有对应目标芯片的内存描述、启动序列和 Flash 下载算法。

下载器内部使用 HPM5301，并不意味着目标芯片也必须是 HPM5301。HPM5301 在这里是
USB 调试探针的主控。

### 9.2 兼容性总表

| 工具 | Arm Cortex-M + SWD | Arm + JTAG | RISC-V + JTAG | 接入方式 |
| --- | ---: | ---: | ---: | --- |
| Keil MDK/µVision | 是 | 视目标和 Pack 而定 | 否 | 原生 CMSIS-DAP Debugger |
| IAR EWARM | 是 | 视目标和版本而定 | 不适用 | 原生 C-SPY CMSIS-DAP |
| IAR for RISC-V | 不适用 | 不适用 | 可通过 GDB Server | OpenOCD/其他 GDB Server |
| OpenOCD | 是 | 是 | 视目标驱动而定 | `adapter driver cmsis-dap` |
| pyOCD | 是 | 以 Cortex-M 支持为主 | 否 | 原生 CMSIS-DAP v1/v2 |
| SEGGER Embedded Studio | 是 | 是 | 视 OpenOCD 目标支持而定 | OpenOCD GDB Server |
| Eclipse/VS Code 等 | 是 | 是 | 视 GDB Server 而定 | OpenOCD/pyOCD + GDB |

“探针可以被 IDE 识别”不等于“IDE 一定能烧录该目标芯片”。烧录还依赖目标芯片
支持包和 Flash 算法。

### 9.3 Keil MDK/µVision

Keil MDK 可以原生使用这些 CMSIS-DAP 下载器调试和烧录 Arm 目标。

建议使用 MDK 5.29 或以上版本。Arm 官方说明：MDK 5.28 及更早版本可能无法识别
CMSIS-DAP v2 复合设备中的 WinUSB 接口，该问题在 5.29 及以后版本修复。

配置步骤：

1. 打开 `Project -> Options for Target -> Debug`；
2. 在调试器下拉框中选择 `CMSIS-DAP Debugger`；
3. 点击 `Settings`，选择对应的 MicroLink、HSLink 或 YBLINK；
4. 选择 `SW` 或 `JTAG` 接口并设置时钟；
5. 打开 `Options for Target -> Utilities`；
6. 启用 `Use Target Driver for Flash Programming`；
7. 选择 `CMSIS-DAP Debugger`；
8. 在 `Settings` 中添加目标芯片对应的 Flash Algorithm；
9. 如需启动调试时自动下载，启用 `Update Target before Debugging`。

目标芯片的 `.FLM` 通常来自 Keil Device Family Pack。没有正确算法时，可能可以读写
RAM 和调试 CPU，但不能擦写片内或外部 Flash。

Keil MDK 主要面向 Arm 目标，不能因为探针支持 JTAG 就直接用它调试 RISC-V MCU。

### 9.4 IAR Embedded Workbench for Arm

IAR EWARM 提供原生 C-SPY CMSIS-DAP 驱动，并支持 CMSIS-DAP v2 的 WinUSB 传输。

配置步骤：

1. 打开 `Project -> Options -> Debugger`；
2. 在 `Setup -> Driver` 中选择 `CMSIS-DAP`；
3. 在 `Debugger -> Download` 中启用 `Verify download`；
4. 启用 `Use flash loader(s)`；
5. 在 `CMSIS-DAP -> Interface` 中选择 SWD 或 JTAG；
6. 设置复位策略、接口时钟和探针序列号；
7. 确认项目使用了正确的 IAR Flash Loader 或 `.board` 配置。

IAR 使用自己的设备描述和 Flash Loader 体系。不能假定把 Keil `.FLM` 直接复制给 IAR
就能使用。

### 9.5 IAR Embedded Workbench for RISC-V

IAR for RISC-V 3.40.x 官方文档列出的 C-SPY 驱动包括：

- Simulator；
- I-jet/I-jet Trace；
- GDB Server；
- 第三方驱动。

其文档没有列出 EWARM 中的原生 CMSIS-DAP C-SPY 驱动。因此使用 HPM5301
CMSIS-DAP 探针调试 RISC-V 目标时，应采用：

```text
IAR for RISC-V
      │
      └── C-SPY GDB Server Driver
              │
              └── OpenOCD
                      │
                      └── CMSIS-DAP v2 探针
```

能否工作还取决于 OpenOCD 是否支持具体 RISC-V SoC、Flash 和复位序列。

### 9.6 OpenOCD

OpenOCD 官方 `cmsis-dap` 驱动支持：

- CMSIS-DAP v1：USB HID；
- CMSIS-DAP v2：USB Bulk/WinUSB；
- SWD；
- JTAG。

典型配置为：

```tcl
adapter driver cmsis-dap
cmsis-dap backend usb_bulk
transport select swd
adapter speed 10000
```

JTAG 模式可改为：

```tcl
transport select jtag
```

还必须加载目标芯片对应的 OpenOCD target/flash 配置。多只 CMSIS-DAP 同时连接时，
应通过 VID/PID、产品名称或序列号选择具体设备。

### 9.7 pyOCD

pyOCD 原生支持 CMSIS-DAP v1 和 v2，主要用于 Arm Cortex-M。它可以从
CMSIS Device Family Pack 中取得目标描述和 FLM Flash 算法。例如：

```bash
pyocd flash \
  --pack=Vendor.Device_DFP.pack \
  --target=device_name \
  firmware.hex
```

pyOCD 不适合作为通用 RISC-V JTAG 后端。

### 9.8 SEGGER Embedded Studio、Eclipse 和 VS Code

SEGGER Embedded Studio 对第三方探针通常采用 GDB Server 接口：

1. `Debug -> Debugger -> Target Connection` 选择 `GDB Server`；
2. GDB Server 类型选择 OpenOCD；
3. 设置 `openocd` 路径和 `-f` 配置文件；
4. 启用自动启动 GDB Server。

Eclipse、VS Code、CLion 等也可以采用相同结构：

```text
IDE/GDB Client -> OpenOCD 或 pyOCD -> CMSIS-DAP -> SWD/JTAG -> 目标 MCU
```

### 9.9 离线下载与 IDE 集成的区别

MicroLink 的离线下载、U 盘拖拽和 PikaPython 功能不依赖 Keil 或 IAR：

- **在线模式**：IDE 通过 CMSIS-DAP 控制探针，IDE 提供 Flash 算法；
- **离线模式**：下载器从自身存储读取固件、脚本和 FLM 算法；
- **U 盘模式**：操作系统复制文件，下载器自主解析并烧录。

同一台硬件可以同时提供这些模式，但需要处理 USB 复合设备接口、目标互斥和任务状态。
例如，IDE 正在调试时不应允许按键同时启动离线烧录。

### 9.10 自制固件的 USB 兼容要求

要让 Windows 和 IDE 稳定识别自制 CMSIS-DAP v2 固件，应至少正确实现：

- USB Bulk 端点；
- CMSIS-DAP v2 接口描述；
- 唯一且稳定的 USB 序列号；
- Microsoft OS/WCID 描述符；
- WinUSB Compatible ID；
- CMSIS-DAP 推荐的设备接口 GUID `{CDB3B5AD-293B-4663-AA36-1AAE46463776}`。

Windows 8 及以上在描述符正确时通常不需要单独安装驱动。Linux 通常需要配置 udev
权限规则。

## 10. 推荐实现路线

### 10.1 目标仅为在线下载调试器

优先选择：

```text
HSLink Pro 硬件 + CherryDAP HSLink-Pro 固件 + HSLinkNexus
```

理由：

- 软硬件和上位机授权明确；
- 已包含电平转换、目标供电和保护；
- 有生产文件和制作指南；
- 已包含 Bootloader 和 UF2 升级。

### 10.2 目标为最小、低成本调试器

可选：

```text
YBLINK 软硬件
```

或者：

```text
自制最小 HPM5301 板 + CherryDAP HSLink-Lite
```

需要明确目标板供电方式和固定 3.3 V 接口风险。

### 10.3 目标为 MicroLink 式离线下载器

建议在许可证明确的工程上独立增加离线层：

```text
HSLink Pro / CherryDAP
        │
        ├── USB MSC 或其他文件传输接口
        ├── 文件系统和固件存储
        ├── BIN/HEX 解析
        ├── FLM 加载器
        ├── SWD 内存访问与算法执行
        ├── 脚本/配置状态机
        ├── 按键、LED、蜂鸣器和日志
        └── 断电保护及结果校验
```

脚本层有两种选择：

1. 嵌入 PikaPython，获得灵活流程；
2. 使用受限的声明式 JSON/CBOR/自定义配置，减少脚本引擎的安全面。

如果产品用于量产，声明式任务格式通常更容易做签名、权限限制、审计和失败恢复；如果用于研发和售后，Python 的灵活性更高。

## 11. 建议的硬件功能块

完整产品至少应评审以下模块：

1. HPM5301 最小系统、电源去耦、复位、启动模式和下载接口；
2. USB Type-C、CC 电阻、USB HS 差分线、ESD 和浪涌保护；
3. SWD/JTAG 信号缓冲及方向控制；
4. Vref 检测和目标电平转换；
5. 可控 3.3 V/5 V 或可调目标供电；
6. 限流、短路保护、防倒灌和电流检测；
7. UART、DTR、RTS，以及可选 RS-485；
8. 固件和算法文件存储空间；
9. 按键、状态 LED、蜂鸣器；
10. 调试器自身救砖接口和量产测试点。

不能直接从“USB HS 内置 PHY”推断 PCB 很简单。USB HS、80 MHz 调试信号、电平转换和目标供电都需要进行信号完整性与电源完整性检查。

## 12. 软件实现任务拆分

建议按以下顺序实现和验收：

### 阶段 A：基础探针

- HPM SDK 启动和时钟；
- CherryUSB USB HS；
- CMSIS-DAP v2；
- SWD/JTAG；
- 复位控制；
- USB CDC 串口。

### 阶段 B：可靠性

- 看门狗；
- 设置持久化；
- Bootloader；
- UF2 或签名升级包；
- USB/SWD 超时恢复；
- 日志和错误码。

### 阶段 C：离线下载

- 文件存储和文件系统；
- BIN/HEX 解析；
- FLM 元数据解析；
- 目标 RAM 分配；
- FLM 初始化、擦除、编程、校验和退出；
- 芯片识别及算法匹配；
- 按键触发和结果指示。

### 阶段 D：自动化和安全

- Python 或受限任务描述；
- 多镜像和多步骤流程；
- 固件哈希和签名；
- 算法/脚本签名；
- 失败重试与断电恢复；
- 序列号、MAC、密钥等个性化数据；
- 生产记录导出；
- 禁止脚本任意访问下载器自身敏感区域。

## 13. 需要进一步确认的问题

在进入原理图和代码设计前，应明确：

1. 目标芯片仅为 Cortex-M，还是还需要 RISC-V JTAG；
2. 是否必须完全脱离电脑；
3. 每个离线任务最大固件容量；
4. 是否需要一次保存多个目标芯片算法；
5. 是否需要可调目标电压及最大输出电流；
6. 是否需要电气隔离；
7. 是否需要 RS-485、CAN、Ymodem 或自定义串口协议；
8. 是否用于量产，以及是否需要序列号和烧录日志；
9. 是否要求安全启动、加密文件和签名验证；
10. 是否允许使用 PikaPython，还是要求更小的固定状态机。

## 14. 许可证与商用检查清单

发布或生产前至少完成：

- [ ] 每个仓库记录具体 commit，而不是只记录分支名；
- [ ] 保存 Apache-2.0、MIT、BSD-3-Clause 等许可证文本；
- [ ] 检查第三方子模块和复制进仓库的源码；
- [ ] 检查 CMSIS-DAP、CherryUSB、HPM SDK、PikaPython 的许可证；
- [ ] 核实 FLM 和 CMSIS-Pack 下载算法是否允许提取和再分发；
- [ ] 不直接复用没有许可证的 MicroLink 自有代码；
- [ ] 不直接复用没有许可证的硬件文件；
- [ ] 检查 USB VID/PID 的合法分配；
- [ ] 为产品使用自己的名称、USB 标识和序列号策略；
- [ ] 对外宣传时区分“协议兼容”和商标授权。

## 15. 信息可信度说明

本记录采用以下核验方式：

- 检查 GitHub 仓库元数据和许可证识别结果；
- 检查仓库完整文件树，而不只依赖 README；
- 阅读项目 README、构建文件和官方项目文档；
- 对芯片规格优先引用先楫官方产品页面；
- 将项目自述性能与已独立验证的事实分开记录。

以下项目声明尚未在本地硬件上独立复测：

- MicroLink 10 MHz 调试时钟和 10 Mbaud 串口；
- MicroLink 与 J-Link V12 的下载速度对比；
- HSLink Pro 最高 80 MHz SWD/JTAG；
- 各方案在不同目标芯片、线长和电压下的稳定性。

## 16. 主要参考链接

1. [Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)
2. [Aladdin-Wang/MicroBoot](https://github.com/Aladdin-Wang/MicroBoot)
3. [cherry-embedded/CherryDAP](https://github.com/cherry-embedded/CherryDAP)
4. [CherryDAP HPM5301EVKLite](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/hpm5301evklite)
5. [CherryDAP HSLink-Pro](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/HSLink-Pro)
6. [cherry-embedded/HSLink-hardware](https://github.com/cherry-embedded/HSLink-hardware)
7. [HSLink/HSLinkNexus](https://github.com/HSLink/HSLinkNexus)
8. [ref42/YBLINK](https://github.com/ref42/YBLINK)
9. [KotoriProject/miniHslink](https://github.com/KotoriProject/miniHslink)
10. [RCSN/hpm_open_hardware](https://github.com/RCSN/hpm_open_hardware)
11. [HPM5300 系列官方页面](https://www.hpmicro.com/product-center/microcontroller/hpm5300)
12. [CherryDAP 项目文档](https://cherrydap.cherry-embedded.org/)
13. [Arm：CMSIS-DAP v2 WinUSB 配置](https://arm-software.github.io/CMSIS-DAP/latest/dap_firmware.html)
14. [OpenOCD：Debug Adapter Configuration](https://openocd.org/doc/html/Debug-Adapter-Configuration.html)
15. [IAR EWARM：CMSIS-DAP options](https://docs.iar.com/ewarm/10.1x/en/c-spy-debugging/debugger-options/reference-information-on-c-spy-hardware-debugger-driver-options/cmsis-dap-options.html)
16. [IAR RISC-V：C-SPY drivers overview](https://docs.iar.com/ewriscv/3.4x/en/c-spy-debugging/the-iar-c-spy-debugger/c-spy-drivers-overview.html)
17. [pyOCD：Debug probes](https://pyocd.io/docs/debug_probes.html)
