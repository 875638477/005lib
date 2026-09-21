# 新版 HPM5301 下载器硬件与 IO 分配

> 主控：HPM5301IEG1，QFN48（约 29 个 I/O）
> 整理日期：2026-09-21
> 关联：[README.md](README.md)、[scope-and-logic-analyzer.md](scope-and-logic-analyzer.md)

本文是**重新设计硬件**的 IO 方案。封装脚位按 HSLink Pro 原理图中的 HPM5301IEG1 符号核对，布线前再用最新数据手册确认 USB D+/D-、电源和晶振脚。

相对上一稿的变更：

- **PA04 到 PA08 恢复 5301 本机 JTAG**（TDO / TDI / TCK / TMS / TRST），不再给逻辑分析仪
- **UART2 与 MicroLink 对齐：PB08 = TX2，PB09 = RX2**
- **OLED 改到 I2C0**：SCL = PA02，SDA = PA03，RES = PY01
- **逻辑分析仪减到 3 路**：PA09 / PA30 / PA31，仍在 GPIOA

## 1. 新版要做什么

| 功能 | 做法 | 备注 |
| --- | --- | --- |
| CMSIS-DAP v2 | 目标 SWD + JTAG，SPI1 高速时序 | PA26 到 PA29，与 HSLink / YBLINK 同组 |
| 5301 本机 JTAG | PA04 到 PA08 保持原功能 | 给 5301 自己调试 / 烧录，不接目标 |
| 双路 USB 串口 | UART2、UART3 引出 | UART2 脚位对齐 MicroLink |
| 本机 UART0 ISP | PA00 / PA01 不改用途 | 只给 5301 自己用 |
| U 盘拖拽烧录 | USB MSC + 外部 NOR | 文件落在外部 Flash |
| 离线烧录 | 外部 Flash 中的 BIN/HEX/FLM + 按键 | OLED 显示进度 |
| 基础逻辑分析仪 | 3 路 GPIOA，先采后传 | 够看 UART / I2C / 3 线 SPI |
| 状态显示 | 3 线 I2C OLED：SCL、SDA、RES | 128×32 或 128×64 |

USB 复合设备建议：

```text
USB HS
 ├── WinUSB  CMSIS-DAP v2
 ├── CDC0    UART2   （PB08 TX / PB09 RX）
 ├── CDC1    UART3
 └── MSC     外部 SPI Flash 上的 FAT
```

## 2. 为什么外部 Flash 选 SPI，不选 QSPI

HPM5301 的 XPI0 / SPI1 / 高速 DAP 挤在同一组脚上：

| 脚 | XPI0 QSPI | SPI1 | 本方案用途 |
| --- | --- | --- | --- |
| PA27 | CA_SCLK | SCLK | 目标 SWCLK / TCK |
| PA28 | CA_D0 | MISO | 目标 SWDIO / TMS |
| PA29 | CA_D2 | MOSI | 目标 TDI |
| PA26 | CA_D3 | CS0 | 目标 TDO |
| PA30 | CA_D1 | DAT2 | 逻辑分析仪 CH1 |
| PA31 | CA_CS0 | DAT3 | 逻辑分析仪 CH2 |
| PA24 / PA25 | CS1 / DQS | CS2 / CS1 | USB D+/D-，不能外挂 Flash |

若把 W25Q 接到 XPI 四线，高速 DAP 的 SPI1 就没了。U 盘和离线要的是容量，不是 XIP。

**选定：SPI2 + GPIO 片选，型号 W25Q128（16 MB）。**

- 4 线：SCLK / MOSI / MISO / CS
- 片选用 PA10 做 GPIO，SPI2 硬件 CS（PB10）留给目标复位

## 3. 分配原则

1. UART0 只服务 5301 的 ROM ISP / 打印，不接到目标，不进 USB CDC。
2. **UART2 必须是 PB08 TX、PB09 RX**，与 MicroLink 一致。
3. UART3 仍出 PB15 TX、PB14 RX。
4. **PA04 到 PA08 只做 5301 本机 JTAG**，做独立调试座，不接目标、不做 LA。
5. 目标 DAP 仍在 PA26 到 PA29，保留 SPI1。
6. 外部 Flash 走 SPI2。
7. OLED 走 **I2C0（PA02 / PA03）+ PY01 RES**。UART2 占了原来的 I2C2。
8. 逻辑分析仪减到 3 路，全部 GPIOA，一次读 DI。
9. 电平转换用自动方向芯片，省掉 SWDIO_DIR。
10. 目标供电用拨动开关，不占 GPIO。

## 4. 总表

| QFN48 | 信号 | 复用功能 | 本方案 | 方向 | 对外 |
| --- | --- | --- | --- | --- | --- |
| 12 | PA00 | UART0.TXD | 5301 ISP TX | 出 | ISP 座 |
| 11 | PA01 | UART0.RXD | 5301 ISP RX | 入 | ISP 座 |
| 8 | PA02 | I2C0.SCL | OLED SCL | 开漏 | OLED 座 |
| 7 | PA03 | I2C0.SDA | OLED SDA | 开漏 | OLED 座 |
| 1 | PA04 | JTAG.TDO | 5301 本机 TDO | 入 | 5301-JTAG 座 |
| 48 | PA05 | JTAG.TDI | 5301 本机 TDI | 出 | 5301-JTAG 座 |
| 47 | PA06 | JTAG.TCK | 5301 本机 TCK | 出 | 5301-JTAG 座 |
| 46 | PA07 | JTAG.TMS | 5301 本机 TMS | 双向 | 5301-JTAG 座 |
| 2 | PA08 | JTAG.TRST | 5301 本机 TRST | 出 | 5301-JTAG 座 |
| 3 | PA09 | GPIO | LA CH0 | 入 | 逻辑分析仪排针 |
| 4 | PA10 | GPIO | W25Q CS# | 出 | 板上 Flash |
| 44 | PA24 | USB 模拟 | USB D- | 差分 | Type-C |
| 43 | PA25 | USB 模拟 | USB D+ | 差分 | Type-C |
| 42 | PA26 | SPI1.CS0 | 目标 TDO | 入 | 目标调试座 |
| 41 | PA27 | SPI1.SCLK | 目标 SWCLK / TCK | 出 | 目标调试座 |
| 40 | PA28 | SPI1.MISO | 目标 SWDIO / TMS | 双向 | 目标调试座 |
| 39 | PA29 | SPI1.MOSI | 目标 TDI | 出 | 目标调试座 |
| 38 | PA30 | GPIO | LA CH1 | 入 | 逻辑分析仪排针 |
| 37 | PA31 | GPIO | LA CH2 | 入 | 逻辑分析仪排针 |
| 27 | PB08 | UART2.TXD | 用户串口 2 TX | 出 | 目标 UART / 20pin-18 |
| 26 | PB09 | UART2.RXD | 用户串口 2 RX | 入 | 目标 UART / 20pin-20 |
| 25 | PB10 | GPIO | 目标 nRESET | 出 | 目标调试座，开漏 |
| 24 | PB11 | SPI2.SCLK | Flash CLK | 出 | 板上 W25Q |
| 23 | PB12 | SPI2.MISO | Flash DO | 入 | 板上 W25Q |
| 22 | PB13 | SPI2.MOSI | Flash DI | 出 | 板上 W25Q |
| 21 | PB14 | UART3.RXD | 用户串口 3 RX | 入 | UART3 座 |
| 20 | PB15 | UART3.TXD | 用户串口 3 TX | 出 | UART3 座 |
| 19 | PY00 | GPIO | 离线按键 | 入 | 轻触，上拉，低有效 |
| 18 | PY01 | GPIO | OLED RES | 出 | OLED 复位，低有效 |
| 13 | RESETN | 复位 | 5301 复位 | 入 | 本机复位键 |
| 14 | WAKEUP | 唤醒 | 接固定电平 | - | 按手册，勿悬空 |
| 35 / 36 | XTALI / XTALO | 晶振 | 24 MHz | - | 负载电容按手册 |

PY00 / PY01 必须同时配 IOC 和 PIOC。PA24 / PA25 在 EVKLite 上是 USB 模拟脚；若手册另有专用 USB_DP / USB_DM，以手册为准。

## 5. 分组说明

### 5.1 UART0：只给 5301 烧固件

```text
PC USB 转串口 3.3 V
    TX ---- PA01 / UART0.RXD
    RX ---- PA00 / UART0.TXD
    GND --- GND
```

- 2.54 mm：GND、3V3、TX、RX
- **禁止**接到目标 MCU，也**禁止**枚举成 USB 串口
- 与下面 5301-JTAG 座二选一即可更新本机固件，建议两套都留测试点

### 5.2 5301 本机 JTAG（PA04 到 PA08）

这是 **HPM5301 自己的调试口**，不是目标 MCU 的 JTAG。

| 5301 | 功能 | 接到 |
| --- | --- | --- |
| PA06 | TCK | 外部调试器 TCK |
| PA07 | TMS | 外部调试器 TMS |
| PA05 | TDI | 外部调试器 TDI |
| PA04 | TDO | 外部调试器 TDO |
| PA08 | TRST | 外部调试器 TRST，目标侧上拉 |
| RESETN | 系统复位 | 可选接到调试器 SRST |
| 3V3 / GND | 电源地 | 与外部调试器共地 |

8 针建议：`3V3 GND TCK TMS TDI TDO TRST RESETN`。丝印写 `5301-JTAG`，不要和目标 20pin 混用。

本机固件两条路：

1. UART0 ISP（日常）
2. 外部 CMSIS-DAP / J-Link 走这 5 个 JTAG 脚（内核调试）

### 5.3 UART2 / UART3：用户双串口

UART2 按 MicroLink 常用脚：

| 逻辑口 | 5301 | USB | 对齐 |
| --- | --- | --- | --- |
| UART2 | **PB08 TX、PB09 RX** | CDC0 | MicroLink TX2 / RX2 |
| UART3 | PB15 TX、PB14 RX | CDC1 | 第二路 |

```text
目标 MCU TX  ---- 电平转换 ---- PB09 / UART2.RXD
目标 MCU RX  ---- 电平转换 ---- PB08 / UART2.TXD
GND          ------------------- GND
```

- TX 串 22 到 33 Ω
- 第一版可固定 3.3 V，或跟 VTref 做电平转换
- 本版不做 DTR/RTS。若要 ESP32 自动下载，用 PA30 / PA31，LA 减到 1 路

### 5.4 目标 DAPLink：SWD + JTAG

| 目标信号 | 5301 | 外设 |
| --- | --- | --- |
| SWCLK / TCK | PA27 | SPI1.SCLK 或 FGPIO |
| SWDIO / TMS | PA28 | SPI1.MISO / 双向 |
| TDI | PA29 | SPI1.MOSI |
| TDO | PA26 | GPIO 输入 |
| nRESET / SRST | PB10 | 开漏，4.7 kΩ 上拉到 Vref |

20pin 1.27 mm（目标侧，不是 5301-JTAG）：

| 20pin | 信号 | 本板来源 |
| --- | --- | --- |
| 1 | VTref | 目标参考电压输入 |
| 2 | TVCC | 拨动开关后的 3.3 V / 5 V |
| 3 | nTRST | NC |
| 5 | TDI | PA29 |
| 7 | TMS / SWDIO | PA28 |
| 9 | TCK / SWCLK | PA27 |
| 13 | TDO | PA26 |
| 15 | nRESET | PB10 |
| 14 / 16 | UART DTR / RTS | NC |
| 18 | UART2 TX | **PB08** |
| 20 | UART2 RX | **PB09** |
| 4 / 6 / 8 / 10 / 12 | GND | GND |
| 19 | +5 V | 可选，限流 |

### 5.5 外部 SPI Flash：U 盘 + 离线库

```text
HPM5301 SPI2                 W25Q128
  PB11 SCLK  ---------------  CLK
  PB13 MOSI  ---------------  DI  (IO0)
  PB12 MISO  ---------------  DO  (IO1)
  PA10 GPIO  ---------------  CS#
  3V3        ---------------  VCC /HOLD /WP
  GND        ---------------  GND
```

目录约定：

```text
/FIRMWARE/*.hex  *.bin
/ALGO/*.flm
/SCRIPT/*.py
/CONFIG/probe.ini
```

在线调试、MSC 写入、离线任务互斥。

### 5.6 3 线 OLED

UART2 占用 PB08 / PB09 后，I2C2 不能再用。OLED 改到 **I2C0**：

| OLED | 5301 | 说明 |
| --- | --- | --- |
| SCL | **PA02 / I2C0.SCL** | 4.7 kΩ 上拉到 3.3 V |
| SDA | **PA03 / I2C0.SDA** | 4.7 kΩ 上拉到 3.3 V |
| RES | **PY01** | 低复位，启动后拉高 |
| VCC | 3.3 V | 不要 5 V 灌进 IO |
| GND | GND | 共地 |

推荐：0.91 寸 128×32 或 0.96 寸 128×64，SSD1306，地址 `0x3C`。

PY01 初始化：

1. `HPM_IOC` 配 GPIO；
2. `HPM_PIOC` 配 `SOC_GPIO_Y_01`；
3. 再拉 RES。

显示：

```text
第 1 行  USB: DAP CDC MSC
第 2 行  U2:115200  U3:115200
第 3 行  DAP: SWD 10M  /  IDLE
第 4 行  FILE: app.hex  42%
```

### 5.7 3 路逻辑分析仪

PA04 到 PA08 还给本机 JTAG 后，同口只剩 3 个空脚，足够看 UART、I2C 或 CLK+MOSI+MISO。

| 通道 | 脚 | GPIOA 位 |
| --- | --- | --- |
| CH0 | PA09 | 9 |
| CH1 | PA30 | 30 |
| CH2 | PA31 | 31 |

- 每路串 100 到 330 Ω
- 只接 3.3 V CMOS
- 1 到 10 MSa/s、64 Ki 点、先采后传、SUMP / PulseView
- 与 DAP、U 盘写入、离线烧录互斥

6 针：

```text
1 CH0   2 CH1
3 CH2   4 GND
5 GND   6 3V3（可选，仅作上拉，不要给被测板供电）
```

## 6. 连接器一览

| 连接器 | 内容 |
| --- | --- |
| USB Type-C | 5301 USB HS |
| 20pin 1.27 mm | 目标 SWD/JTAG + UART2（PB08/PB09）+ VTref |
| 8pin 5301-JTAG | PA04 到 PA08 + RESETN + 3V3 + GND |
| 4pin UART3 | GND、PB15 TX、PB14 RX、3V3 |
| 4pin ISP | GND、PA00、PA01、3V3 |
| 6pin LA | 3 通道 + GND |
| 5pin OLED | VCC、GND、PA02 SCL、PA03 SDA、PY01 RES |
| 本机复位 | RESETN |
| 离线键 | PY00 |
| 目标电源拨动 | OFF / 3.3 V / 5 V |

两套调试口不要插反：

```text
5301-JTAG 座  -->  调试 HPM5301 自己
目标 20pin    -->  调试用户板
```

## 7. 电气与电源

- 本机 5 V 来自 Type-C，3.3 V 按手册去耦
- USB HS 等长、完整地、插座旁 ESD
- 目标 TVCC 用开关和防倒灌
- 5301-JTAG 与目标 20pin 分地回流，避免两套调试器同时接同一地回路打架
- OLED、W25Q 用本机 3.3 V

## 8. 固件侧对应关系

| 模块 | 外设 / 脚 |
| --- | --- |
| 本机 ISP / 控制台 | UART0，PA00 / PA01 |
| 本机内核调试 | JTAG，PA04 到 PA08（启动后勿改复用） |
| USB CDC0 | UART2，**PB08 / PB09** + HDMA |
| USB CDC1 | UART3，PB15 / PB14 + HDMA |
| 目标 CMSIS-DAP | SPI1 + FGPIO，PA26 到 PA29；复位 PB10 |
| MSC / 离线文件 | SPI2，PB11 到 PB13，CS = PA10 |
| OLED | I2C0，PA02 / PA03；RES = PY01 |
| 离线启动 | PY00 下降沿，消抖 20 ms |
| 逻辑分析仪 | GPIOA 的 PA09 / PA30 / PA31 |

模式互斥：DAP 在线、MSC 写入、离线烧录、LA 采集。CDC 可与 DAP 同时。OLED 始终可刷。

## 9. 引脚冲突与放弃项

| 想加的功能 | 冲突 | 建议 |
| --- | --- | --- |
| XPI QSPI Flash | PA26 到 PA31 = 目标 DAP + LA | 不采用 |
| 8 路 LA | PA04 到 PA08 已还给本机 JTAG | 保持 3 路 |
| UART2 改回 PA08/PA09 | 破坏 MicroLink 对齐，且 PA08 是 TRST | 不采用 |
| OLED 继续用 PB08/PB09 | 与 UART2 冲突 | 改 I2C0 |
| UART2 DTR/RTS | 占 LA 脚 | 需要时减 LA |
| 可调 VTref ADC | 无空模拟脚 | 第一版固定 3.3 V |

## 10. 布线核对清单

- [ ] UART0 丝印 `5301-ISP`
- [ ] PA04 到 PA08 丝印 `5301-JTAG`，不要接到目标 20pin
- [ ] UART2 必须是 PB08 TX、PB09 RX，20pin 的 18/20 与此一致
- [ ] OLED 是 PA02 / PA03 / PY01，不是 PB08 / PB09
- [ ] PA02 / PA03 开漏并上拉 4.7 kΩ
- [ ] PY00 / PY01 同时配 PIOC
- [ ] PA26 到 PA29 按 SPI1 高速走线
- [ ] W25Q 四线短、CS 上拉
- [ ] LA 只有 PA09 / PA30 / PA31
- [ ] 目标 nRESET 开漏
- [ ] USB DP/DM 对手册确认

## 11. 建议实现顺序

1. 最小系统：USB 枚举；UART0 ISP 与 5301-JTAG 都能更新本机
2. 目标 DAP：SWD 再 JTAG
3. CDC0 = UART2（PB08/PB09），再 CDC1 = UART3
4. SPI2 + W25Q + MSC
5. 拖拽 / 离线烧录 + OLED
6. 3 路 GPIO 采样 + SUMP
