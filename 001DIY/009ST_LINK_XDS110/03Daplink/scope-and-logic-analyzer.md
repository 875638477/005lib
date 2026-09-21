# MicroLink 迷你示波器与逻辑分析仪实现说明

> 对应仓库：[Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)
> 补充文档：[MicroBoot / MKLink](https://microboot.readthedocs.io/zh-cn/latest/tools/microlink/microlink/)
> 整理日期：2026-09-21

## 1. 先说结论

MicroLink / MKLink 里的“迷你示波器”和“逻辑分析仪”都不是传统仪器：

| 名称 | 实际采集对象 | 采集路径 | 上位机 |
| --- | --- | --- | --- |
| 迷你示波器 | 目标 MCU RAM 中的变量 | SWD 读内存 -> VOFA+ JustFloat | VOFA+ / J-Scope 类软件 |
| SystemView | RTOS 任务/中断事件 | SWD 读 RTT 通道 -> USB CDC | SystemView |
| RTTView | 目标打印日志 | SWD 读 RTT 通道 -> USB CDC | 任意串口助手 |
| 硬件逻辑分析仪 | GPIO 高低电平边沿 | 无 | 不存在于公开仓库 |

公开的 `Aladdin-Wang/MicroLink` 仓库：

- 有 SWD Host、USB CDC、PikaPython、RTTView、SystemView 入口；
- 有 `jscope.jpg` 这类波形图；
- 没有 GPIO 高速采样、SUMP、PulseView、nanoDLA 一类硬件逻辑分析仪代码；
- README 中的“使用逻辑分析仪测试时钟引脚”指的是**外部仪器**去量 MicroLink 的 SWD/UART 波形。

因此：

- 要复刻“能画曲线的示波器”，应实现 **SWD 变量采样 + VOFA+**；
- 要复刻“能看任务/中断时序”的分析仪，应实现 **RTT + SystemView**；
- 要做真正的 GPIO 逻辑分析仪，必须另做一套采样器和主机协议。

## 2. 为什么看起来像示波器/逻辑分析仪

传统仪器和 MicroLink 的差别：

```text
真正的示波器
  ADC 采样模拟电压 -> 时间/电压波形

真正的逻辑分析仪
  GPIO 采样数字边沿 -> 多通道时序 / 协议解码

MicroLink 迷你示波器
  SWD 读目标变量 -> 把数值当成通道画曲线

MicroLink SystemView
  SWD 读目标 RTT 事件 -> 把任务/ISR 画成时间轴
```

两者都复用了调试器已有的高速 SWD 和 USB HS，不需要额外 ADC 前端或比较器阵列。代价是：

- 看不到真实电气波形、上升沿、过冲、毛刺；
- 看不到未进入目标内存的外部数字信号；
- 采样率受 SWD 带宽、目标是否 halt、RTT 缓冲大小限制。

## 3. 迷你示波器：SWD + VOFA+

### 3.1 数据通路

```text
目标 MCU RAM 变量
        │
        │ SWD 存储器访问
        ▼
HPM5301 固件周期读
        │
        │ 转成 float / JustFloat 帧
        ▼
USB CDC 虚拟串口
        │
        ▼
VOFA+ 画曲线
```

官方文档的表述是：MKLink 通过 SWD 直接读取目标芯片内存中的变量，封装成 VOFA+ 协议，再经 USB CDC 发给 PC。不占用目标 USART，也不要求目标程序主动 `printf`。

这和 J-Link J-Scope 同类：探针当“内存示波器”，不是模拟前端。

### 3.2 启动命令

文档给出两种用法。

连续读取若干 `float`：

```python
vofa.send(0x20000030, 5, 0.00001)
```

含义：

- `0x20000030`：第一个变量地址；
- `5`：连续读取 5 个 `float`；
- `0.00001`：周期，单位秒，最小约 1 us；
- 周期设为 `0` 停止。

按类型读取：

```python
vofa.send(
    0x20000030, "uint8_t",
    0x2000154c, "uint16_t",
    0x20001550, "float",
    0.00001
)
```

限制：

- 一次最多约 16 个通道；
- 变量必须 4 字节对齐，否则会读到撕裂数据；
- 地址通常从 `.map` / AXF 符号表取得。

公开 GitHub 仓库的 `main.py` 已导入 `RTTView`、`SystemView`，但尚未导入 `vofa`。VOFA+ 命令出现在后续 MKLink 文档和固件中，实现思路与 RTT/SystemView 相同：PikaPython 命令入口 + SWD 周期读。

### 3.3 JustFloat 帧格式

VOFA+ 常用 JustFloat：

```text
[float0 4字节小端] [float1 4字节小端] ... [floatN 4字节小端] [00 00 80 7F]
```

帧尾 `00 00 80 7F` 是小端 `+Inf`（`0x7F800000`）。

固件侧伪代码：

```c
float ch[N];
uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F};

for (i = 0; i < N; i++) {
    swd_read_memory(addr[i], &raw, sizeof(raw));
    ch[i] = convert_to_float(raw, type[i]);
}
usb_cdc_write(ch, sizeof(ch));
usb_cdc_write(tail, 4);
```

### 3.4 固件实现要点

周期任务建议独立于 CMSIS-DAP 调试会话：

1. 解析 `vofa.send(...)`；
2. 保存地址、类型、周期；
3. 用 `swd_init_debug()` 保持目标可访问，尽量不 halt CPU；
4. 用 `swd_read_memory()` / `swd_read_word()` 读变量；
5. 转成 float 后按 JustFloat 发送；
6. 周期为 0 或 USB 断开时停止。

公开仓库已有对应底层：

- `MicroLink/microlink_app/src/swd_host/swd_host.c`
- `swd_read_memory()`、`swd_read_word()`
- `MicroLink/microlink_app/src/USB2Python/usb2python.c`

高速采样时注意：

- 优先 32-bit 对齐访问；
- 多个相邻 `float` 一次 burst 读取；
- USB CDC 用环形缓冲，避免阻塞 SWD；
- 与在线调试、离线烧录互斥或分时；
- 文档标称最高约 1 M 读取速率，实际取决于 SWD 时钟、通道数和 USB 调度。

### 3.5 目标程序侧

最小要求只是变量存在且地址稳定：

```c
__attribute__((aligned(4))) volatile float vofa_ch[5];

void loop(void)
{
    vofa_ch[0] = current;
    vofa_ch[1] = voltage;
    vofa_ch[2] = speed;
}
```

目标不必实现 VOFA+。协议由探针封装。

## 4. 逻辑分析仪：公开仓库里实际是什么

### 4.1 `10M_TTL.jpg` 不是 MicroLink 用 IO 做逻辑分析仪

[10M_TTL.jpg](https://github.com/Aladdin-Wang/MicroLink/blob/main/images/microlink/10M_TTL.jpg) 容易被看成“MicroLink 用 IO 口实现逻辑分析仪”，因为它有通道、阈值、脉宽和时间轴。画面实际内容是：

- 窗口标题：`LA2016已连接 - KingstVIS`
- 设备型号：`LA2016`
- 采样：`1 GSa / 200 MHz`
- 左侧设置：`I/O电平标准 3.3V CMOS -> Vth=1.65 V`
- 通道 0 解码：`UART / RS232 / 485`
- 右侧测量：脉宽 `100 ns`，占空比 `50%`，频率 `5 MHz`
- 游标：`A1 - A2 = 100 ns`

这是**康芯微 Kingst LA2016** 的上位机。`I/O电平标准` 是外部逻辑分析仪探头的输入阈值，用来告诉 LA2016：“被测信号是 3.3 V CMOS，比较门限 1.65 V”。它不是 MicroLink 固件里的 GPIO 采集配置。

README 原文也写明了用途：

> 使用逻辑分析仪抓取波形如图所示，每个bit传输的时间为 1/10M = 100ns。

上一张图 `10M_Baud.jpg` 是串口助手以 10 Mbaud 收发 `0x55`。`0x55` 的比特是 `01010101`，所以 TTL 波形接近 5 MHz 方波，每个 bit 100 ns。测试关系是：

```text
PC 串口助手
    │ USB CDC
    ▼
MicroLink UART TX  （被测对象，输出 10 Mbaud）
    │ 飞线接到外部探头
    ▼
Kingst LA2016
    │
    ▼
KingstVIS 显示通道 0 并做 UART 解码
```

同目录的 `clk.jpg` 也是外部仪器：Tektronix 示波器在测 SWD CLK，时基 100 ns/div。这些图用来证明 MicroLink **自己能发出** 10 MHz SWD 和 10 Mbaud UART，不是证明它内置逻辑分析仪。

### 4.2 没有 GPIO 硬件逻辑分析仪

在 `Aladdin-Wang/MicroLink` 的文件树中，未找到：

- SUMP / OLS；
- PulseView / libsigrok 驱动；
- nanoDLA / FX2 / CY7C68013；
- 高速 GPIO 采样状态机；
- 多通道边沿触发器。

仓库里的 ADC 头文件来自 HPM SDK 和 PikaPython 标准设备库，不是示波器前端。

README 中的逻辑分析仪图片，是用外部仪器测量 MicroLink 自己的 SWD CLK 和 10 Mbaud UART。

### 4.3 最接近“分析仪”的是 SystemView

SystemView 把 RTOS 事件画成时间轴，看起来像软件逻辑分析仪：

```text
目标 RTOS
  任务切换 / ISR / 标记
        │
        │ 写入 RTT UpBuffer，通常通道 1
        ▼
HPM5301 用 SWD 轮询 RTT 控制块
        │
        ▼
USB CDC
        │
        ▼
SEGGER SystemView 或 MKLink 上位机
```

公开仓库证据：

`MicroLink/external/pikapython/main.py`：

```python
import RTTView
import SystemView
```

`SystemView.pyi`：

```python
def start(self, *val): ...
def stop(self, *val): ...
```

`usb2python.c` 把 USB 字符先交给 RTT，再交给 SystemView，最后才交给 Python：

```c
if (write_rtt_and_receive_usb(ch) == 0) {
    if (SYSVIEW_REC_ProcessIncoming(ch) == 0) {
        if (pikaMain != NULL) {
            obj_runChar(pikaMain, ch);
        }
    }
}
```

启动命令：

```python
SystemView.start(0x20000000, 1024, 1)
```

含义：

- `0x20000000`：搜索 `_SEGGER_RTT` 控制块的起始地址；
- `1024`：搜索范围；
- `1`：SystemView 使用的 RTT 通道。

RTT 日志则是：

```python
RTTView.start(0x20000000, 1024)
```

### 4.4 RTT / SystemView 采集原理

目标 MCU 中的 RTT 控制块大致为：

```text
_SEGGER_RTT
  ├── aUp[n]    目标 -> 主机
  └── aDown[n]  主机 -> 目标
```

探针循环：

1. 按地址范围扫描 `"SEGGER RTT"` 标识；
2. 读取 `WrOff` / `RdOff`；
3. 把新数据从目标 RAM 拷到本地；
4. 更新 `RdOff`；
5. 从 USB 收到的下行数据写入 `aDown`。

SystemView 只是把通道 1 的二进制事件流转发给上位机，由上位机解码：

- 任务创建/切换；
- ISR 进入/退出；
- 用户标记；
- CPU 占用。

这能分析软件时序，不能替代 GPIO 逻辑分析仪去抓 I2C、SPI、UART 或毛刺。

## 5. 公开源码对应关系

| 模块 | 路径 | 作用 |
| --- | --- | --- |
| Python 命令通道 | `microlink_app/src/USB2Python/usb2python.c` | USB CDC1 <-> PikaPython / RTT / SystemView |
| SWD 主机 | `microlink_app/src/swd_host/swd_host.c` | 读目标内存、寄存器、Flash 算法 |
| SystemView 脚本接口 | `external/pikapython/SystemView.pyi` | `start` / `stop` |
| 启动脚本 | `external/pikapython/main.py` | 导入 RTTView、SystemView、FLM、Ymodem |
| VOFA+ | 后续 MKLink 固件/文档 | SWD 周期读变量并封装 JustFloat |

`usb2python.c` 依赖 `SEGGER_RTTView.h`、`SEGGER_SystemView.h`。公开树里能看到调用，完整采集实现可能在未完全开源的固件中。复刻时应自行实现 RTT 扫描和环形缓冲读写，不要直接复制 SEGGER 专有源码。

## 6. 若要做成真正的硬件逻辑分析仪

不能沿用 VOFA+/SystemView 这条路。需要单独的数字采集通道。

### 6.1 硬件

- 8 到 16 路高速 GPIO 或比较器；
- 与 SWD 引脚隔离或分时复用；
- 足够 SRAM 或外部 SRAM/HyperRAM；
- USB HS 连续上传；
- 输入保护、限幅、可选阈值调节。

HPM5301 适合 USB HS 上传，但 GPIO 采样深度和触发能力有限，高速率通道数会很快碰到 SRAM 和 USB 带宽上限。

### 6.2 常见软件协议

| 协议 | 上位机 | 特点 |
| --- | --- | --- |
| SUMP / OLS | PulseView、sigrok、OLS | 实现简单，先采后传 |
| 自定义流式协议 | 自研上位机 | 可持续上传，固件更复杂 |
| nanoDLA / FX2 兼容 | PulseView | 常见开源 LA，主控通常不是 HPM5301 |

最小 SUMP 流程：

```text
上位机下发采样率、通道、触发、深度
        │
        ▼
固件等触发并写入样本缓冲
        │
        ▼
采集结束后经 USB/UART 回传
        │
        ▼
PulseView 解码 I2C/SPI/UART 等
```

### 6.3 和调试器共存

SWD 下载与 GPIO 采样会争用 CPU、DMA、USB 和引脚。建议：

1. 模式互斥：调试 / 示波器 / 逻辑分析仪；
2. 或分时：停采后再允许 DAP；
3. 不要在 USB MSC 写文件时做高速采集。

## 7. 建议的实现顺序

如果当前调试、串口、离线下载已经可用，按这个顺序加“示波器/分析仪”：

1. 打通 `swd_read_memory()` 在目标运行时读 RAM；
2. 实现 RTT 控制块扫描和通道 0 转发，得到 RTTView；
3. 增加 VOFA+ JustFloat 周期采样，得到迷你示波器；
4. 增加 RTT 通道 1 转发，对接 SystemView；
5. 若仍需要电气级数字时序，再单独做 GPIO LA。

第 3 步就能覆盖 MicroLink 宣传的“迷你示波器”。第 4 步覆盖“软件逻辑分析仪”。第 5 步才是传统逻辑分析仪。

## 8. 验证方法

### 8.1 VOFA+ 示波器

1. 目标定义对齐的 `float` 数组并周期更新；
2. 从 `.map` 取地址；
3. 打开 VOFA+，协议选 JustFloat；
4. 向 MicroLink 虚拟串口发送 `vofa.send(...)`；
5. 确认通道数量、数值范围、周期与目标变量一致；
6. 停止命令后曲线不再更新；
7. 同时开 Keil 调试，确认不会互相卡死。

### 8.2 SystemView

1. 目标集成 RTT + SystemView，使用通道 1；
2. 发送 `SystemView.start(addr, range, 1)`；
3. 上位机应看到任务切换和 ISR；
4. 目标不跑 RTOS 时，不应误报为硬件 LA 失败。

### 8.3 排除硬件 LA 误判

如果 PulseView 无法识别设备，这是预期现象：公开 MicroLink 不是 SUMP 设备。

## 9. 参考

1. [Aladdin-Wang/MicroLink](https://github.com/Aladdin-Wang/MicroLink)
2. [MicroLink / MKLink 文档](https://microboot.readthedocs.io/zh-cn/latest/tools/microlink/microlink/)
3. [VOFA+ JustFloat](https://www.vofa.plus/docs/learning/dataengines/justfloat)
4. `MicroLink/microlink_app/src/USB2Python/usb2python.c`
5. `MicroLink/microlink_app/src/swd_host/swd_host.h`
6. `MicroLink/external/pikapython/main.py`
7. [sigrok SUMP](https://sigrok.org/wiki/SUMP_compatibles)
