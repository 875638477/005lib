# DAPLink / CMSIS-DAP 下载器详细说明

> 目标存放位置：`D:\001DIY\009ST_LINK_XDS110\03Daplink`  
> 整理日期：2026-09-21  
> 内容范围：基本原理、开源方案、HPM5301 实现、Keil/IAR/OpenOCD 接入、硬件设计、固件架构、离线烧录和验证方法

## 1. 结论

如果要自行制作一个能够在 Keil、IAR、OpenOCD 等工具中使用的通用下载器，建议采用：

```text
HPM5301
  + CherryDAP/CMSIS-DAP v2
  + USB CDC 串口
  + HSLink Pro 开源硬件
```

如果还需要类似 MicroLink 的离线烧录功能，再增加：

```text
USB MSC/文件系统
  + BIN/HEX 解析
  + FLM 下载算法加载器
  + 脚本或任务配置
  + 按键/LED/蜂鸣器
```

各方案推荐程度：

| 需求 | 推荐方案 |
| --- | --- |
| 直接在开发板验证 CMSIS-DAP | CherryDAP HPM5301EVKLite |
| 完整开源软硬件、可生产 | HSLink Pro |
| 最小化、学习 Rust 实现 | YBLINK |
| U 盘和脚本化离线烧录 | 参考 MicroLink 的架构，重新实现离线层 |
| Keil/IAR 调试 Arm MCU | CMSIS-DAP v2 |
| RISC-V 调试 | CMSIS-DAP JTAG + OpenOCD GDB Server |

## 2. 名词区别

### 2.1 CMSIS-DAP

CMSIS-DAP 是 Arm 定义的主机与调试探针之间的通信协议。

数据链路为：

```text
Keil / IAR / OpenOCD / pyOCD
              │
              │ USB HID 或 USB Bulk
              ▼
        CMSIS-DAP 调试器
              │
              │ SWD 或 JTAG
              ▼
           目标 MCU
```

CMSIS-DAP 主要有两种 USB 传输方式：

| 版本 | USB 方式 | 特点 |
| --- | --- | --- |
| CMSIS-DAP v1 | HID | 兼容性好、无需专用驱动，速度较低 |
| CMSIS-DAP v2 | Bulk/WinUSB | 速度高，适合 HPM5301 的 USB HS |

CMSIS-DAP 规定的是通信协议，不限定调试器主控。主控可以是 STM32、NXP、HPM5301、ESP32-S3 等。

### 2.2 DAPLink

DAPLink 是 Arm Mbed 基于 CMSIS-DAP 开发的一套完整调试器固件，通常还包括：

- CMSIS-DAP 调试；
- USB 虚拟串口；
- USB U 盘拖拽下载；
- Bootloader；
- 板卡识别和固件升级。

因此：

```text
CMSIS-DAP = 协议
DAPLink   = 实现该协议的一套固件/产品框架
CherryDAP = 使用 CherryUSB 实现的轻量化 CMSIS-DAP/DAPLink 模板
```

### 2.3 CherryDAP

CherryDAP 使用 CherryUSB 和 CMSIS-DAP 代码构建高速调试器，支持：

- CMSIS-DAP 2.1；
- SWD；
- JTAG；
- USB 转串口；
- WebUSB/自定义 HID；
- HPM5301EVKLite；
- HSLink Pro/HSLink Lite。

项目：

- [cherry-embedded/CherryDAP](https://github.com/cherry-embedded/CherryDAP)

### 2.4 MicroLink

MicroLink 是 HPM5301 多功能下载器，在 CMSIS-DAP 基础上增加：

- USB 转串口；
- RTT 转虚拟串口；
- USB U 盘；
- BIN/HEX 拖拽下载；
- FLM 下载算法；
- PikaPython 脚本；
- 离线烧录；
- Ymodem 文件发送；
- 自身固件升级。

项目：

- [Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)

注意：MicroLink 根仓库当前没有统一 LICENSE，且没有完整可编辑 PCB 工程，不应直接视为授权完整的开源产品。

## 3. 与 ST-Link、XDS110 的区别

| 项目 | ST-Link | XDS110 | CMSIS-DAP/DAPLink |
| --- | --- | --- | --- |
| 主要厂商/生态 | STMicroelectronics | Texas Instruments | Arm 标准/社区 |
| 主机协议 | ST 专用协议 | TI 专用调试协议 | CMSIS-DAP 标准 |
| 最适合目标 | STM32 | TI MCU/DSP | Arm Cortex-M，部分 JTAG 场景 |
| Keil | 支持 | 通常不作为标准探针 | 原生支持 |
| IAR EWARM | 支持 | 依具体驱动 | 原生支持 |
| OpenOCD | 支持 | 依版本和目标 | 原生 `cmsis-dap` 驱动 |
| 固件开放程度 | 主要为厂商固件 | 主要为厂商方案 | CMSIS-DAP、DAPLink、CherryDAP 可开源 |
| 自制难度 | 受固件和授权限制 | 受生态和固件限制 | 可基于开源工程自行实现 |

不能仅通过修改 USB 名称，把 CMSIS-DAP 变成 ST-Link 或 XDS110。三者主机协议不同。

## 4. HPM5301 为什么适合做调试器

HPM5301 的优势包括：

- 单核 32 位 RISC-V；
- 片内 Flash 和较大 SRAM；
- 内置 USB High-Speed PHY；
- 高速 GPIO；
- SPI、UART、DMA 等外设；
- QFN48 封装，适合小型调试器；
- 官方 HPM SDK 和开发板支持。

需要注意主频描述：

- HPM5300 系列官方资料写“最高 480 MHz”；
- HPM5301/HPM5301EVKLite 的部分官方资料标注 360 MHz；
- MicroLink 自述 HPM5301 工作于 480 MHz。

自行设计时必须以实际料号最新数据手册、勘误表和额定工作条件为准，不要把 USB 的 480 Mbps 与 CPU 480 MHz 混为一谈。

## 5. 可用的开源方案

### 5.1 HSLink Pro

这是最适合直接参考或二次开发的方案。

项目地址：

- 固件：[CherryDAP/projects/HSLink-Pro](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/HSLink-Pro)
- 硬件：[cherry-embedded/HSLink-hardware](https://github.com/cherry-embedded/HSLink-hardware)
- 上位机：[HSLink/HSLinkNexus](https://github.com/HSLink/HSLinkNexus)

主要功能：

- HPM5301；
- CMSIS-DAP 2.1；
- SWD/JTAG；
- GPIO/SPI 时序输出；
- USB 转串口；
- DTR/RTS；
- Vref 检测和电平转换；
- 可控目标供电；
- UF2 Bootloader；
- 上位机配置和升级。

开放资料：

- KiCad 原理图；
- KiCad PCB；
- 原理图 PDF；
- BOM；
- 坐标文件；
- 生产归档；
- 制作指南；
- 设备固件；
- 上位机源码。

许可证：

- 固件：Apache-2.0；
- 硬件：Apache-2.0；
- 上位机：Apache-2.0；
- HPM SDK：BSD-3-Clause。

### 5.2 YBLINK

项目：

- [ref42/YBLINK](https://github.com/ref42/YBLINK)

特点：

- HPM5301；
- 纯 Rust 固件；
- CMSIS-DAP v2；
- USB High-Speed；
- FGPIO 模拟 SWD/JTAG；
- CDC ACM 串口；
- 提供预编译固件；
- 提供 Altium 原理图、PCB、Gerber。

许可证：

- MIT/Apache-2.0。

适合学习最小实现，但不包含目标供电、电平转换和离线烧录。

主要引脚：

| 信号 | HPM5301 引脚 |
| --- | --- |
| SWCLK/TCK | PA27 |
| SWDIO/TMS | PA28 |
| TDI | PA29 |
| TDO | PA26 |
| nRESET | PB10 |
| UART TX | PA00 |
| UART RX | PA01 |

### 5.3 CherryDAP HPM5301EVKLite

项目：

- [CherryDAP HPM5301EVKLite](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/hpm5301evklite)
- [项目文档](https://cherrydap.cherry-embedded.org/projects/HPM5301EVKLite)

适合先使用 HPM5301EVKLite 开发板验证：

- USB 枚举；
- CMSIS-DAP v2；
- SWD；
- JTAG；
- USB 转串口；
- Keil/IAR/OpenOCD 识别。

建议先在开发板完成软件验证，再设计定制 PCB。

### 5.4 MicroLink

项目：

- [Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)

公开软件目录包含：

| 目录 | 功能 |
| --- | --- |
| `external/DAP` | CMSIS-DAP、SWD、JTAG |
| `middleware/cherryusb` | USB Device 协议栈 |
| `src/USB2MSC` | USB U 盘 |
| `src/USB2UART` | USB 串口 |
| `src/USB2Python` | Python 命令通道 |
| `external/pikapython` | PikaPython 引擎 |
| `vfs/drag-n-drop` | 拖拽文件和 Intel HEX |
| `vfs/target` | Flash 算法和目标编程 |
| `src/swd_host` | 目标 SWD 访问 |
| `src/ymodem_send` | Ymodem 发送 |

硬件目录当前只有原理图 PDF和 Altium Preview 缓存，没有完整的 SchDoc、PcbDoc、Gerber 和 BOM。

## 6. Keil MDK/µVision 接入

### 6.1 版本要求

建议使用 Keil MDK 5.29 或以上版本。

MDK 5.28 及更早版本可能无法识别 CMSIS-DAP v2 复合 USB 设备中不位于接口 0 的 WinUSB 接口。

### 6.2 调试设置

1. 连接 CMSIS-DAP 下载器和目标板；
2. 打开 `Project -> Options for Target -> Debug`；
3. 调试器选择 `CMSIS-DAP Debugger`；
4. 点击 `Settings`；
5. 选择正确的探针；
6. `Port` 选择 `SW` 或 `JTAG`；
7. 设置调试时钟；
8. 确认能够看到目标 SW-DP/JTAG-DP。

建议首次连接从较低时钟开始，例如 1 MHz，再逐步提高。

### 6.3 Flash 下载设置

1. 打开 `Options for Target -> Utilities`；
2. 选择 `Use Target Driver for Flash Programming`；
3. 驱动选择 `CMSIS-DAP Debugger`；
4. 点击 `Settings`；
5. 在 `Programming Algorithm` 中添加目标芯片对应的 `.FLM`；
6. 根据需求启用 `Erase`、`Program`、`Verify`；
7. 如需进入调试时自动烧录，启用 `Update Target before Debugging`。

如果没有正确 `.FLM`，可能可以连接 CPU 和读写 RAM，但无法烧写 Flash。

### 6.4 Keil 的限制

- MDK 主要支持 Arm 目标；
- 探针支持 JTAG 不代表 Keil 可以调试 RISC-V；
- 目标芯片必须存在 Device Family Pack 或自定义设备支持；
- 外部 QSPI/OSPI Flash 通常需要专用 FLM；
- 复位方式需要与目标硬件匹配。

## 7. IAR Embedded Workbench 接入

### 7.1 IAR EWARM

IAR Embedded Workbench for Arm 提供原生 CMSIS-DAP 驱动。

设置步骤：

1. 打开 `Project -> Options -> Debugger`；
2. `Setup -> Driver` 选择 `CMSIS-DAP`；
3. 打开 `Debugger -> Download`；
4. 启用 `Verify download`；
5. 启用 `Use flash loader(s)`；
6. 打开 `CMSIS-DAP -> Interface`；
7. 选择 SWD 或 JTAG；
8. 设置接口速度和复位方式；
9. 选择正确探针序列号；
10. 检查目标芯片对应的 IAR Flash Loader。

IAR 使用自己的 Flash Loader 和 `.board` 配置。Keil `.FLM` 不能默认直接用于 IAR。

### 7.2 IAR for RISC-V

IAR for RISC-V 的官方 C-SPY 驱动列表未包含 EWARM 的原生 CMSIS-DAP 驱动。

建议结构：

```text
IAR for RISC-V
      │
      └── C-SPY GDB Server Driver
              │
              └── OpenOCD
                      │
                      └── CMSIS-DAP v2/JTAG
                              │
                              └── RISC-V 目标
```

是否能够烧录取决于 OpenOCD 是否支持具体 RISC-V SoC、Flash 控制器和复位流程。

## 8. OpenOCD 接入

### 8.1 SWD 示例

```tcl
adapter driver cmsis-dap
cmsis-dap backend usb_bulk
transport select swd
adapter speed 10000

source [find target/stm32f4x.cfg]
```

启动：

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg
```

或者使用包含上述设置的自定义配置文件。

### 8.2 JTAG 示例

```tcl
adapter driver cmsis-dap
cmsis-dap backend usb_bulk
transport select jtag
adapter speed 10000

source [find target/目标芯片.cfg]
```

### 8.3 GDB 连接

```bash
arm-none-eabi-gdb firmware.elf
```

GDB 中执行：

```gdb
target extended-remote localhost:3333
monitor reset halt
load
monitor reset run
```

RISC-V 目标需要使用相应的 RISC-V GDB。

## 9. pyOCD 接入

pyOCD 支持 CMSIS-DAP v1/v2，主要用于 Arm Cortex-M。

安装设备包后可使用：

```bash
pyocd flash \
  --pack=Vendor.Device_DFP.pack \
  --target=device_name \
  firmware.hex
```

启动 GDB Server：

```bash
pyocd gdbserver \
  --pack=Vendor.Device_DFP.pack \
  --target=device_name
```

pyOCD 可以从 CMSIS Device Family Pack 中加载目标描述和 FLM 算法，但不适合作为通用 RISC-V 调试后端。

## 10. SEGGER Embedded Studio、VS Code、Eclipse

这些 IDE 可以通过 OpenOCD 或 pyOCD 的 GDB Server 使用 CMSIS-DAP。

通用结构：

```text
IDE
 │
 └── GDB Client
       │
       └── OpenOCD/pyOCD GDB Server
             │
             └── CMSIS-DAP
                   │
                   └── SWD/JTAG
```

SEGGER Embedded Studio 设置：

1. `Debug -> Debugger -> Target Connection` 选择 `GDB Server`；
2. GDB Server 类型选择 `OpenOCD`；
3. 指定 `openocd.exe`；
4. 填写 OpenOCD 配置文件；
5. 启用 `Auto Start GDB Server`；
6. 默认 GDB 端口通常为 3333。

## 11. 自制 CMSIS-DAP v2 的 USB 要求

为了让 Windows 和 IDE 稳定识别，需要正确实现：

- USB Bulk IN/OUT 端点；
- CMSIS-DAP v2 接口描述符；
- 正确的端点包长；
- 稳定且唯一的序列号；
- Microsoft OS/WCID 描述符；
- WinUSB Compatible ID；
- CMSIS-DAP 推荐的接口 GUID：
  `{CDB3B5AD-293B-4663-AA36-1AAE46463776}`。

Windows 8 及以上在描述符正确时通常无需单独安装驱动。

Linux 需要通过 udev 规则赋予普通用户 USB 访问权限。

## 12. HPM5301 调试器硬件组成

### 12.1 最小系统

至少包括：

- HPM5301IEG1；
- 芯片要求的电源和去耦；
- 复位电路；
- 启动模式配置；
- 自身 JTAG/ISP 下载接口；
- 时钟配置；
- 测试点。

### 12.2 USB

包括：

- USB Type-C；
- CC1/CC2 下拉电阻；
- USB HS D+/D-；
- ESD 防护；
- 必要的共模和串联阻尼位置；
- USB VBUS 检测；
- 电源输入保护。

USB HS 差分对需要：

- 控制差分阻抗；
- 等长；
- 少过孔；
- 连续参考平面；
- 远离高噪声开关节点。

### 12.3 SWD/JTAG

需要的目标接口信号：

- SWDIO/TMS；
- SWCLK/TCK；
- TDI；
- TDO；
- nRESET/SRST；
- 可选 nTRST；
- Vref；
- GND。

推荐预留：

- 源端串联电阻；
- 电平转换旁路；
- 上拉/下拉配置；
- 示波器/逻辑分析仪测试点；
- 每个高速信号相邻地线。

### 12.4 电平转换

如果仅支持固定 3.3 V，可以简化电路，但不能安全连接 1.8 V 等目标。

通用调试器应支持：

- Vref 输入检测；
- 根据 Vref 设置 SWD/JTAG 输出电平；
- 探针未连接目标时禁用输出；
- 掉电目标防反向灌电；
- 方向快速切换，尤其是 SWDIO。

### 12.5 目标供电

若由探针给目标板供电，应具有：

- 可控开关；
- 限流；
- 短路保护；
- 过温保护；
- 防倒灌；
- 电压、电流检测；
- 软件默认关闭或安全状态；
- 清晰区分 Vref 输入和 Tvcc 输出。

Vref 与 Tvcc 不应在目标板上随意短接。

### 12.6 串口

建议提供：

- UART TX/RX；
- DTR；
- RTS；
- 可选方向控制；
- 可选 RS-485；
- 电平跟随 Vref 或明确标注固定电平。

## 13. 固件模块划分

推荐目录结构：

```text
firmware/
├── board/                  # HPM5301 板级、Pinmux、时钟
├── bootloader/             # UF2/安全升级/救砖
├── cmsis_dap/              # DAP 命令
├── swd/                    # SWD 时序
├── jtag/                   # JTAG 时序
├── usb/
│   ├── dap_v2/             # USB Bulk
│   ├── cdc/                # USB 串口
│   ├── msc/                # 可选 U 盘
│   └── descriptors/        # USB 描述符和 WCID
├── target_power/           # 电源、电平和复位控制
├── settings/               # 参数持久化
├── offline/
│   ├── filesystem/         # 固件文件
│   ├── hex/                # Intel HEX
│   ├── flm/                # FLM 加载
│   ├── scripts/            # Python/任务配置
│   └── programmer/         # 离线编程状态机
└── app/
    └── main.c
```

建议采用任务或状态机隔离：

- USB DAP；
- USB 串口；
- 离线下载；
- 文件系统；
- 升级；
- LED/按键；
- 看门狗。

在线调试和离线下载必须互斥，防止两个控制源同时操作 SWD/JTAG。

## 14. MicroLink 式离线烧录原理

### 14.1 所需文件

典型离线任务包括：

```text
offline_download.py
target_algorithm.FLM.o
application.bin
或 application.hex
```

### 14.2 执行流程

```text
按键/命令触发
    │
    ├── 读取任务配置
    ├── 检查固件哈希
    ├── 连接目标并读取芯片 ID
    ├── 选择 FLM
    ├── 下载 FLM 到目标 RAM
    ├── 调用 Init
    ├── 擦除目标 Flash
    ├── 分块写入固件
    ├── 回读或算法校验
    ├── 调用 UnInit
    ├── 复位目标
    └── 保存日志并显示结果
```

### 14.3 FLM 的作用

FLM 是在目标 MCU RAM 中执行的 Flash 编程算法，通常提供：

- `Init`；
- `UnInit`；
- `EraseChip`；
- `EraseSector`；
- `ProgramPage`；
- 可选校验功能。

下载器负责：

1. 解析算法元数据；
2. 分配目标 RAM；
3. 写入算法代码和参数；
4. 设置寄存器、栈和入口；
5. 启动目标 CPU；
6. 等待函数返回；
7. 读取返回值；
8. 处理超时和故障。

### 14.4 安全和可靠性

必须处理：

- 目标型号和算法不匹配；
- 目标 RAM 不足；
- 地址越界；
- 算法死循环；
- USB 断开；
- 目标掉电；
- 烧录中途断电；
- 文件损坏；
- 错误脚本；
- 失败重试；
- 唯一序列号写入；
- 日志追溯。

量产设备建议对固件、算法和任务配置进行签名，不建议允许任意脚本访问下载器自身敏感区域。

## 15. 开发顺序

### 第一阶段：开发板验证

- HPM5301EVKLite；
- CherryDAP；
- CMSIS-DAP v2 枚举；
- Keil 识别；
- SWD 下载 STM32；
- JTAG 连接测试；
- USB 串口环回。

### 第二阶段：定制硬件

- 参考 HSLink Pro；
- 完成 HPM5301 最小系统；
- USB HS；
- SWD/JTAG；
- 复位；
- UART；
- Vref 和电平转换；
- 目标供电保护。

### 第三阶段：可靠性

- Bootloader；
- UF2 或升级包；
- 看门狗；
- 设置持久化；
- USB 异常恢复；
- SWD/JTAG 超时；
- 救砖接口。

### 第四阶段：离线功能

- 文件系统；
- BIN/HEX；
- FLM；
- 按键触发；
- LED/蜂鸣器；
- 日志；
- 多任务配置；
- 安全签名。

## 16. 验证清单

### 16.1 USB

- [ ] Windows 正确显示 CMSIS-DAP；
- [ ] 不需要手工安装 WinUSB 驱动；
- [ ] 每台设备序列号唯一；
- [ ] USB CDC 正常；
- [ ] USB 插拔 100 次无死机；
- [ ] 休眠/唤醒正常；
- [ ] 多只探针能够按序列号区分。

### 16.2 Keil/IAR

- [ ] Keil 能识别探针；
- [ ] Keil 能看到目标 DAP；
- [ ] Keil 擦除、编程、校验成功；
- [ ] Keil 单步、断点、变量正常；
- [ ] IAR 能识别 CMSIS-DAP；
- [ ] IAR Flash Loader 正常；
- [ ] 复位后连接；
- [ ] Connect under reset。

### 16.3 SWD/JTAG

- [ ] 100 kHz 连接；
- [ ] 1 MHz 连接；
- [ ] 10 MHz 连接；
- [ ] 目标低压测试；
- [ ] 长线测试；
- [ ] 错接保护；
- [ ] 目标掉电不反向供电；
- [ ] JTAG 链多 TAP 测试。

### 16.4 串口

- [ ] 常见波特率；
- [ ] 高波特率；
- [ ] 长时间无丢包；
- [ ] DTR/RTS；
- [ ] 调试下载时串口无乱码；
- [ ] 目标掉电时 IO 安全。

### 16.5 离线下载

- [ ] BIN；
- [ ] HEX；
- [ ] 多个 Flash 区域；
- [ ] 芯片 ID 校验；
- [ ] 全片擦除；
- [ ] 扇区擦除；
- [ ] 编程；
- [ ] 校验；
- [ ] 失败重试；
- [ ] 中途断电恢复；
- [ ] 日志记录；
- [ ] 错误算法拒绝运行。

## 17. 常见问题

### IDE 看不到探针

检查：

1. Keil 是否不低于 5.29；
2. 设备管理器中是否存在 WinUSB 设备；
3. USB 描述符/WCID 是否正确；
4. VID/PID 和接口号；
5. 是否被其他程序独占；
6. USB 线是否只有充电功能；
7. 固件是否提供稳定序列号。

### 能识别探针但找不到目标

检查：

1. GND 是否共地；
2. Vref 是否正确；
3. SWDIO/SWCLK 是否接反；
4. nRESET 是否需要连接；
5. 目标是否供电；
6. 时钟是否过高；
7. 是否需要 Connect under reset；
8. 目标读保护状态；
9. 电平转换是否启用。

### 能调试但不能烧写 Flash

检查：

1. 是否安装目标 Device Pack；
2. 是否添加正确 FLM/Flash Loader；
3. Flash 地址和容量；
4. RAM for Algorithm；
5. 外部 Flash 初始化；
6. 目标写保护；
7. 算法是否匹配芯片版本。

### OpenOCD 找不到 CMSIS-DAP

检查：

1. `adapter driver cmsis-dap`；
2. `cmsis-dap backend usb_bulk`；
3. Windows WinUSB；
4. Linux udev 权限；
5. 其他程序是否占用探针；
6. 多探针时是否指定序列号。

## 18. 许可证和商用注意事项

正式生产前应检查：

- [ ] 记录每个依赖的具体 commit；
- [ ] 保留 Apache-2.0、MIT、BSD-3-Clause 许可证；
- [ ] 检查 CherryUSB、CMSIS-DAP、HPM SDK 等第三方组件；
- [ ] 检查仓库子模块；
- [ ] 不直接复用没有许可证的 MicroLink 自有代码；
- [ ] 不直接生产没有许可证的硬件设计；
- [ ] 核实 FLM/CMSIS-Pack 是否允许提取和再分发；
- [ ] 合法取得 USB VID/PID；
- [ ] 使用自己的 USB 产品名和序列号规则；
- [ ] 不冒用 ST-Link、J-Link、XDS110 等商标或 USB 标识。

## 19. 推荐最终方案

建议将项目分为两个型号。

### 基础型

```text
HPM5301
+ CherryDAP
+ CMSIS-DAP v2
+ SWD/JTAG
+ USB CDC
+ UF2 Bootloader
+ 固定 3.3 V 或简单 Vref
```

适合研发和个人使用。

### 完整型

```text
基础型
+ 全电压电平转换
+ 可控目标供电
+ 电流/短路保护
+ DTR/RTS/RS-485
+ USB MSC
+ FLM 动态加载
+ BIN/HEX 离线下载
+ 脚本/任务配置
+ 固件签名
+ 生产日志
```

适合量产、售后和多目标芯片维护。

## 20. 参考资料

1. [Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)
2. [Aladdin-Wang/MicroBoot](https://github.com/Aladdin-Wang/MicroBoot)
3. [cherry-embedded/CherryDAP](https://github.com/cherry-embedded/CherryDAP)
4. [CherryDAP HPM5301EVKLite](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/hpm5301evklite)
5. [CherryDAP HSLink-Pro](https://github.com/cherry-embedded/CherryDAP/tree/master/projects/HSLink-Pro)
6. [cherry-embedded/HSLink-hardware](https://github.com/cherry-embedded/HSLink-hardware)
7. [HSLink/HSLinkNexus](https://github.com/HSLink/HSLinkNexus)
8. [ref42/YBLINK](https://github.com/ref42/YBLINK)
9. [RCSN/hpm_open_hardware](https://github.com/RCSN/hpm_open_hardware)
10. [HPM5300 系列官方页面](https://www.hpmicro.com/product-center/microcontroller/hpm5300)
11. [Arm CMSIS-DAP 固件说明](https://arm-software.github.io/CMSIS-DAP/latest/dap_firmware.html)
12. [OpenOCD Debug Adapter Configuration](https://openocd.org/doc/html/Debug-Adapter-Configuration.html)
13. [IAR EWARM CMSIS-DAP options](https://docs.iar.com/ewarm/10.1x/en/c-spy-debugging/debugger-options/reference-information-on-c-spy-hardware-debugger-driver-options/cmsis-dap-options.html)
14. [pyOCD Debug probes](https://pyocd.io/docs/debug_probes.html)

