# 新版 HPM5301 下载器硬件与 IO 分配

> 主控：HPM5301IEG1，QFN48（约 29 个 I/O）
> 整理日期：2026-09-21
> 关联：[README.md](README.md)、[scope-and-logic-analyzer.md](scope-and-logic-analyzer.md)

本文是**重新设计硬件**的 IO 方案，不是 MicroLink / HSLink 原板抄脚。封装脚位按 HSLink Pro 原理图中的 HPM5301IEG1 符号核对，布线前再用最新数据手册确认 USB D+/D-、电源和晶振脚。

## 1. 新版要做什么

| 功能 | 做法 | 备注 |
| --- | --- | --- |
| CMSIS-DAP v2 | SWD + JTAG，SPI1 高速时序 | 与 HSLink / YBLINK 同一组脚 |
| 双路 USB 串口 | UART2、UART3 引出 | 两路独立 CDC |
| 本机固件烧录 | UART0 保持 ISP，不改用途 | 只给 5301 自己用 |
| U 盘拖拽烧录 | USB MSC + 外部 NOR | 文件落在外部 Flash |
| 离线烧录 | 外部 Flash 中的 BIN/HEX/FLM + 按键 | OLED 显示进度 |
| 基础逻辑分析仪 | 8 路 GPIOA，先采后传 | 见示波器/分析仪文档第 6 节 |
| 状态显示 | 3 线 I2C OLED：SCL、SDA、RES | 128×32 或 128×64 |

USB 复合设备建议：

```text
USB HS
 ├── WinUSB  CMSIS-DAP v2
 ├── CDC0    UART2
 ├── CDC1    UART3
 └── MSC     外部 SPI Flash 上的 FAT
```

RTT / Python 命令若还要第三路虚拟串口，可以从 CDC1 分时，或放弃 MSC 同时挂第三路 CDC。QFN48 上不要再加第四路物理 UART。

## 2. 为什么外部 Flash 选 SPI，不选 QSPI

HPM5301 的 XPI0 / SPI1 / 高速 DAP 挤在同一组脚上：

| 脚 | XPI0 QSPI | SPI1 | 本方案用途 |
| --- | --- | --- | --- |
| PA27 | CA_SCLK | SCLK | 目标 SWCLK / TCK |
| PA28 | CA_D0 | MISO | 目标 SWDIO / TMS |
| PA29 | CA_D2 | MOSI | 目标 TDI |
| PA26 | CA_D3 | CS0 | 目标 TDO |
| PA30 | CA_D1 | DAT2 | 逻辑分析仪 CH6 |
| PA31 | CA_CS0 | DAT3 | 逻辑分析仪 CH7 |
| PA24 / PA25 | CS1 / DQS | CS2 / CS1 | USB D+/D-，不能外挂 Flash |

若把 W25Q 接到 XPI 四线，高速 DAP 的 SPI1 就没了，SWD 只能 GPIO 位bang。U 盘和离线烧录要的是容量，不是 XIP 执行。

**选定：SPI2 + GPIO 片选，型号 W25Q128（16 MB）。**

- 4 线：SCLK / MOSI / MISO / CS
- 时钟 50 到 80 MHz + HDMA，拖几兆 HEX 足够
- 不占用 PA26 到 PA31
- 片选用 PA10 做 GPIO，把 SPI2 硬件 CS（PB10）留给目标复位

8 MB 的 W25Q64 也能用，建议直接上 16 MB，好放多个目标固件和 FLM。

## 3. 分配原则

1. UART0 只服务 5301 自己的 ROM ISP / 调试打印，不接到目标板，不进 USB CDC。
2. 用户串口只出 UART2、UART3。
3. 目标调试脚固定在 PA26 到 PA29，保留 SPI1 模拟 SWD/JTAG。
4. 外部 Flash 走 SPI2，避开 DAP。
5. OLED 走 I2C2，避开 UART 和 Flash。
6. 8 路逻辑分析仪全部放在 GPIOA，一次读一个 DI 寄存器。
7. PA04 到 PA07 是 5301 自己的 JTAG。本机固件改走 UART0 ISP，这 4 脚改作 LA，不再做 5301 的 JTAG 插座。
8. 电平转换用自动方向芯片（如 TXS0102 / TXS0108），省掉 SWDIO_DIR。
9. 目标供电用拨动开关，不占 GPIO。

27 个可用 GPIO 全部占满。多一个功能就要减一个现有功能。

## 4. 总表

| QFN48 | 信号 | 复用功能 | 本方案 | 方向 | 对外 |
| --- | --- | --- | --- | --- | --- |
| 12 | PA00 | UART0.TXD | 5301 ISP TX | 出 | ISP 座，接 USB 转串口 RX |
| 11 | PA01 | UART0.RXD | 5301 ISP RX | 入 | ISP 座，接 USB 转串口 TX |
| 8 | PA02 | GPIO | LA CH0 | 入 | 逻辑分析仪排针 |
| 7 | PA03 | GPIO | LA CH1 | 入 | 逻辑分析仪排针 |
| 1 | PA04 | JTAG.TDO | LA CH2 | 入 | 逻辑分析仪排针 |
| 48 | PA05 | JTAG.TDI | LA CH3 | 入 | 逻辑分析仪排针 |
| 47 | PA06 | JTAG.TCK | LA CH4 | 入 | 逻辑分析仪排针 |
| 46 | PA07 | JTAG.TMS | LA CH5 | 入 | 逻辑分析仪排针 |
| 2 | PA08 | UART2.TXD | 用户串口 2 TX | 出 | UART2 座 / 20pin-18 |
| 3 | PA09 | UART2.RXD | 用户串口 2 RX | 入 | UART2 座 / 20pin-20 |
| 4 | PA10 | GPIO | W25Q CS# | 出 | 板上 Flash，低有效 |
| 44 | PA24 | USB 模拟 | USB D- | 差分 | Type-C |
| 43 | PA25 | USB 模拟 | USB D+ | 差分 | Type-C |
| 42 | PA26 | SPI1.CS0 | 目标 TDO | 入 | 调试座 |
| 41 | PA27 | SPI1.SCLK | 目标 SWCLK / TCK | 出 | 调试座 |
| 40 | PA28 | SPI1.MISO | 目标 SWDIO / TMS | 双向 | 调试座 |
| 39 | PA29 | SPI1.MOSI | 目标 TDI | 出 | 调试座 |
| 38 | PA30 | GPIO | LA CH6 | 入 | 逻辑分析仪排针 |
| 37 | PA31 | GPIO | LA CH7 | 入 | 逻辑分析仪排针 |
| 27 | PB08 | I2C2.SCL | OLED SCL | 开漏 | OLED 座 |
| 26 | PB09 | I2C2.SDA | OLED SDA | 开漏 | OLED 座 |
| 25 | PB10 | GPIO | 目标 nRESET | 出 | 调试座，开漏 |
| 24 | PB11 | SPI2.SCLK | Flash CLK | 出 | 板上 W25Q |
| 23 | PB12 | SPI2.MISO | Flash DO / IO1 | 入 | 板上 W25Q |
| 22 | PB13 | SPI2.MOSI | Flash DI / IO0 | 出 | 板上 W25Q |
| 21 | PB14 | UART3.RXD | 用户串口 3 RX | 入 | UART3 座 |
| 20 | PB15 | UART3.TXD | 用户串口 3 TX | 出 | UART3 座 |
| 19 | PY00 | GPIO | 离线按键 | 入 | 轻触，上拉，低有效 |
| 18 | PY01 | GPIO | OLED RES | 出 | OLED 复位，低有效 |
| 13 | RESETN | 复位 | 5301 复位 | 入 | 本机复位键 |
| 14 | WAKEUP | 唤醒 | 接固定电平 | - | 按手册，勿悬空 |
| 35 / 36 | XTALI / XTALO | 晶振 | 24 MHz | - | 负载电容按手册 |
| 18 / 19 之外 | PY00 / PY01 | PIOC | 必须同时配 IOC 和 PIOC | - | 见第 8 节 |

电源、地、DCDC、USB 专用脚以数据手册为准，上表只列 GPIO 与本方案有关的脚。PA24 / PA25 在 EVKLite 上配成 USB 模拟脚；若手册另有专用 USB_DP / USB_DM，则以手册为准，这两脚不要再当 GPIO。

## 5. 分组说明

### 5.1 UART0：只给 5301 烧固件

```text
PC USB 转串口 3.3 V
    TX ---- PA01 / UART0.RXD
    RX ---- PA00 / UART0.TXD
    GND --- GND
```

- 2.54 mm 4 针：GND、3V3、TX、RX，或只出 GND / TX / RX
- 进入 ISP：本机复位 + BOOT/KEY 时序，或拉 BOOT 脚（若板子另做拨码）
- **禁止**把 UART0 接到目标 MCU，也**禁止**把它枚举成 USB 串口
- 量产后可用测试点，开发阶段建议保留插座

### 5.2 UART2 / UART3：用户双串口

| 逻辑口 | 5301 | USB | 典型用途 |
| --- | --- | --- | --- |
| UART2 | PA08 TX、PA09 RX | CDC0 | 目标日志、ESP32 下载 |
| UART3 | PB15 TX、PB14 RX | CDC1 | 第二路日志或 RS-485 |

两路都要：

- 电平转换到目标 Vref，或第一版固定 3.3 V
- TX 串 22 到 33 Ω
- 与目标共地
- 本版**不做 DTR/RTS**。QFN48 没有余脚。若必须自动下载 ESP32，从 LA 减去 2 路，用 PA30 / PA31 作 UART2 的 DTR / RTS

### 5.3 DAPLink：SWD + JTAG

与 YBLINK / HSLink 保持同一组，方便沿用 SPI 时序：

| 目标信号 | 5301 | 外设 |
| --- | --- | --- |
| SWCLK / TCK | PA27 | SPI1.SCLK 或 FGPIO |
| SWDIO / TMS | PA28 | SPI1.MISO / 双向 |
| TDI | PA29 | SPI1.MOSI |
| TDO | PA26 | GPIO 输入 / SPI1.CS 脚当输入 |
| nRESET / SRST | PB10 | 开漏，4.7 kΩ 上拉到 Vref |
| nTRST | 不引出 | QFN48 无余脚；需要时用飞线到测试点 |

20pin 1.27 mm 建议按 J-Link / HSLink 习惯：

| 20pin | 信号 | 本板来源 |
| --- | --- | --- |
| 1 | VTref | 目标参考电压输入 |
| 2 | TVCC | 拨动开关后的 3.3 V / 5 V，可选 |
| 3 | nTRST | NC |
| 5 | TDI | PA29 |
| 7 | TMS / SWDIO | PA28 |
| 9 | TCK / SWCLK | PA27 |
| 13 | TDO | PA26 |
| 15 | nRESET | PB10 |
| 14 / 16 | UART DTR / RTS | NC（本版） |
| 18 | UART2 TX | PA08 |
| 20 | UART2 RX | PA09 |
| 4 / 6 / 8 / 10 / 12 | GND | GND |
| 19 | +5 V | 可选，限流 |

SWD 最小只要：VTref、SWDIO、SWCLK、nRESET、GND。JTAG 再加上 TDI、TDO。

### 5.4 外部 SPI Flash：U 盘 + 离线库

```text
HPM5301 SPI2                 W25Q128
  PB11 SCLK  ---------------  CLK
  PB13 MOSI  ---------------  DI  (IO0)
  PB12 MISO  ---------------  DO  (IO1)
  PA10 GPIO  ---------------  CS#
  3V3        ---------------  VCC
  GND        ---------------  GND
  3V3  10k   ---------------  /HOLD /WP（若有，拉高）
```

- 紧贴 5301，CLK 等长，不要过孔菊花链
- 上电 CS# 保持高
- 文件系统：片内 Flash 只放 5301 固件；用户镜像、FLM、脚本全部放 W25Q
- U 盘：CherryUSB MSC，底层块设备指向这片 Flash 的 FAT 分区
- 离线：按 PY00，从约定目录取最新 HEX/BIN + FLM，经 SWD/JTAG 写入目标
- 在线调试与 MSC 写入、离线任务必须互斥

目录约定（可再改，固件按此实现）：

```text
/FIRMWARE/*.hex  *.bin
/ALGO/*.flm
/SCRIPT/*.py
/CONFIG/probe.ini
```

### 5.5 3 线 OLED

用户指定 SCL、SDA、RES，即 I2C OLED + 复位，不是 SPI 三线屏。

| OLED | 5301 | 说明 |
| --- | --- | --- |
| SCL | PB08 / I2C2.SCL | 4.7 kΩ 上拉到 3.3 V |
| SDA | PB09 / I2C2.SDA | 4.7 kΩ 上拉到 3.3 V |
| RES | PY01 | 低复位，启动后拉高 |
| VCC | 3.3 V | 不要 5 V 灌进 IO |
| GND | GND | 共地 |

推荐模组：0.91 寸 128×32 或 0.96 寸 128×64，控制器 SSD1306，地址 `0x3C`。线长小于 10 cm。

PY01 属于 PY 口，初始化必须：

1. `HPM_IOC` 配成 GPIO；
2. `HPM_PIOC` 配成 `SOC_GPIO_Y_01`；
3. 再当普通 GPIO 翻转 RES。

显示内容（基础即可）：

```text
第 1 行  USB: DAP CDC MSC
第 2 行  U2:115200  U3:115200
第 3 行  DAP: SWD 10M  /  IDLE
第 4 行  FILE: app.hex  42%
```

逻辑分析仪工作时改显示采样率、深度、触发脚。没有独立 LED 脚，状态以 OLED 为准，可另加电源指示发光管（不占 GPIO）。

### 5.6 8 路逻辑分析仪

全部在 GPIOA，一次读 `GPIOA->DI`，再按位打包：

| 通道 | 脚 | 包装位 |
| --- | --- | --- |
| CH0 | PA02 | 2 |
| CH1 | PA03 | 3 |
| CH2 | PA04 | 4 |
| CH3 | PA05 | 5 |
| CH4 | PA06 | 6 |
| CH5 | PA07 | 7 |
| CH6 | PA30 | 30 |
| CH7 | PA31 | 31 |

- 每路串 100 到 330 Ω，排针旁放 ESD
- 只接 3.3 V CMOS，5 V 或未共地不要进脚
- 第一版 1 到 10 MSa/s、64 Ki 点、先采后传、SUMP / PulseView
- 与 DAP、U 盘写入、离线烧录互斥
- PA04 到 PA07 兼 5301 JTAG：调试 5301 内核时拔掉 LA 排线，日常只靠 UART0 ISP

10 针排针建议：

```text
1 CH0   2 CH1
3 CH2   4 CH3
5 CH4   6 CH5
7 CH6   8 CH7
9 GND  10 GND
```

## 6. 连接器一览

| 连接器 | 内容 |
| --- | --- |
| USB Type-C | 5301 USB HS，CC 5.1 kΩ |
| 20pin 1.27 mm | 目标 SWD/JTAG + UART2 + VTref |
| 4pin UART3 | GND、TX、RX、3V3（可选） |
| 4pin ISP | GND、PA00、PA01、3V3 |
| 10pin LA | 8 通道 + 2×GND |
| 5pin OLED | VCC、GND、SCL、SDA、RES |
| 本机复位 | RESETN |
| 离线键 | PY00 |
| 目标电源拨动 | OFF / 3.3 V / 5 V，经防倒灌到 TVCC |

W25Q 只在 PCB 上，不对外。

## 7. 电气与电源

- 本机 5 V 来自 Type-C VBUS，3.3 V LDO 或 5301 内部 DCDC，按手册去耦
- USB HS：D+/D- 等长、参考地完整、ESD 靠近插座
- 目标 TVCC 用开关，额定电流写在丝印上，串保险或限流
- VTref 只作电平转换参考，第一版可不做 ADC 检测
- 目标复位、SWD、UART 与 LA **分地回流**，LA 不要跟 SWCLK 平行长走线
- 5301 与 W25Q、OLED 共 3.3 V 本机电源

## 8. 固件侧对应关系

| 模块 | 外设 / 脚 |
| --- | --- |
| 本机 ISP / 控制台 | UART0，PA00 / PA01 |
| USB CDC0 | UART2，PA08 / PA09 + HDMA |
| USB CDC1 | UART3，PB15 / PB14 + HDMA |
| CMSIS-DAP | SPI1 + FGPIO，PA26 到 PA29；复位 PB10 |
| MSC / 离线文件 | SPI2，PB11 到 PB13，CS = PA10 |
| OLED | I2C2，PB08 / PB09；RES = PY01（IOC+PIOC） |
| 离线启动 | PY00 下降沿，消抖 20 ms |
| 逻辑分析仪 | GPIOA 输入，HDMA 读 DI；缓冲 64 KiB |

模式互斥：

```text
IDLE
 ├── DAP 在线调试
 ├── CDC 串口（可与 DAP 同时）
 ├── MSC 主机正在写 U 盘
 ├── 离线烧录（按键或脚本）
 └── 逻辑分析仪采集
```

OLED 始终可刷新。采集或烧录时 MSC 只读或断开写入。

## 9. 引脚冲突与放弃项

| 想加的功能 | 冲突 | 建议 |
| --- | --- | --- |
| XPI QSPI Flash | PA26 到 PA31 = DAP + LA | 不采用 |
| UART2 DTR/RTS | 无空脚 | 减 2 路 LA，或第二版换 LQFP64 料号 |
| 可调 VTref ADC | PB 模拟脚已给 Flash / 串口 / OLED | 第一版固定 3.3 V |
| SWDIO_DIR | 无空脚 | 自动方向电平转换 |
| WS2812 / 状态灯 GPIO | 无空脚 | 用 OLED；电源灯不接 MCU |
| 5301 专用 JTAG 插座 | PA04 到 PA07 = LA CH2 到 CH5 | 只用 UART0 ISP |
| 16 路 LA | 脚不够 | 保持 8 路 |

换 HPM5321 / 5331 的 LQFP64 才能同时要 QSPI、DTR/RTS、VTref ADC 和 5301 JTAG 插座。本方案锚定 **HPM5301IEG1 QFN48**。

## 10. 布线核对清单

- [ ] UART0 只到 ISP 座，丝印写 `5301-ISP`，不要写 `UART`
- [ ] UART2 / UART3 丝印写清 TX 是 5301 输出
- [ ] PA26 到 PA29 按 SPI1 高速规则走，并给自动方向电平转换留位
- [ ] W25Q 四线短、CS 上拉
- [ ] OLED 上拉 4.7 kΩ，RES 接到 PY01 不是 PB 其它脚
- [ ] PY00 / PY01 同时配 PIOC
- [ ] LA 每路串联电阻，地针夹在排针两端
- [ ] PA04 到 PA07 不要再做 5301 JTAG 插座
- [ ] USB DP/DM 按最新手册确认是 PA24/PA25 还是专用脚
- [ ] 目标 nRESET 开漏，不要推挽灌 5 V

## 11. 建议实现顺序

1. 最小系统：USB 枚举、UART0 ISP 能更新 5301
2. DAP：SWD 再 JTAG，对接 Keil / OpenOCD
3. CDC：先 UART2，再 UART3
4. SPI2 + W25Q：能读 ID，再做 FAT 和 MSC
5. 拖拽 HEX/BIN + FLM 在线烧目标
6. 按键离线烧录 + OLED 显示
7. 8 路 GPIO 采样 + SUMP

第 1 到 3 步就能当 HSLink 类调试器。第 4 到 6 步才是 MicroLink 的 U 盘和离线。第 7 步是额外仪器功能。
