# HPM6280 与 HPM6284（BGA116）能否共用外部 NOR 启动

记录日期：2026-09-26

结论先说：**BGA116 上做不到完全替换。** HPM6284 不能改从板载外部 NOR 启动，程序也不能烧进这颗外部 Flash。两颗芯片封装焊球可以按同一 BGA116 使用，但启动介质和烧录目标不同。

## 问题

- HPM6280 没有片内 Flash。
- HPM6284 有 4MB 片内 Flash。
- 两者都有 7×7 BGA116（订购型号后缀 IEP，例如 HPM6280IEP、HPM6284IEP）。
- 问：在已经贴了外部 NOR 的板上，能否让 HPM6284 也从外部 Flash 启动，并把程序烧到外部 Flash，从而两颗芯片完全互换。

## 结论

| 项目 | HPM6280（BGA116） | HPM6284（BGA116） |
| --- | --- | --- |
| 片内 Flash | 无 | 4MB，合封（SIP） |
| 启动介质 | 只能是外部 NOR | 只能是片内 4MB |
| 外部 NOR 接在哪 | XPI0 CA（BGA116 唯一引出的 XPI 端口） | 不能用 CA 外接存储器 |
| 烧录目标 | 外部 NOR | 片内 Flash |
| 与另一颗互换 | 不能按“同一外部 Flash、同一烧录流程”替换 | 同左 |

同一块板若必须两颗都能跑：

- 外部 NOR 仍要贴在 XPI0 CA 上，给 HPM6280 用。
- 换上 HPM6284 时，按片内 Flash 重新配置并烧进片内。外面那颗 NOR 不会成为启动设备，烧录器对它也写不进去。
- HPM6284 不要再把 CA 这些脚（PA00–PA07）当成 XPI 去驱动外部 NOR。若这些脚改作 GPIO，而外部 NOR 仍焊在上面，要避免总线冲突；不用时保持高阻即可。

## 依据

### 1. 片内 Flash 占用的是内部第二组引脚，不是封装上的 CA

《HPM6000 系列 MCU 片上 Flash 使用指南》对带 4MB 片内闪存的型号（含 **HPM6284Ixx1**、HPM6264Ixx1）说明：

- SIP 用 XPI0 的**第二组引脚 PX** 连接内部 Flash。这组脚只对 SIP 有效，没有引到封装外。
- 因为片内 Flash 走的是 XPI0 CA 这一路，封装上对应的 XPI0 CA 复用功能失效。这些脚只能当 GPIO、UART 等，不能再配成 XPI0。
- 明确限制：**不可以再配置 XPI0 的 CA 端口，也不可以用 XPI0 CA 连接外部器件。**
- 仍可使用 XPI0 的 **CB** 端口，或 XPI1（若该系列有）去接外部串行存储器。

HPM6284 上 XPI 功能失效的 CA 脚：

| 引脚 | 信号 |
| --- | --- |
| PA00 | XPI0.CA_CS0 |
| PA06 | XPI0.CA_CS1 |
| PA04 | XPI0.CA_SCLK |
| PA03 | XPI0.CA_D[0] |
| PA01 | XPI0.CA_D[1] |
| PA02 | XPI0.CA_D[2] |
| PA05 | XPI0.CA_D[3] |
| PA07 | XPI0.CA_DQS |

BootROM 认片内 Flash 时，镜像里的 NOR 配置选项要把引脚组选到内部第二组。先楫 FAQ 的写法：

- `option[2] = 0x00001000`：选用内部 Flash（`pin_group_sel = 1`，第二组）
- `option[2] = 0x0`：默认，使用外部 Flash（第一组）

`0x00001000` 的 bit12 对应配置结构里的 `pin_group_sel`。片上 Flash 指南里的示例还会把头字段从 `0xfcf90001` 改成 `0xfcf90002`，让选项字覆盖到这一字段。量产烧录时目标也要选片内 Flash / 内部引脚组，而不是外部 CA。

### 2. BGA116 只引出 XPI0 CA，没有 CB 可以绕开

《HPM6200 数据手册》Rev2.5 表 44「封装引出功能差异」：

| 订购后缀 | 封装 | XPI |
| --- | --- | --- |
| HPM62xxxPAx | 144 eLQFP，20×20 | 1 路 XPI（CA 和 CB 都引出） |
| HPM62xxxPBx | 100 eLQFP，14×14 | 1 路，**XPI0 仅 CA** |
| HPM62xxxEPx | 116 BGA，7×7 | 1 路，**XPI0 仅 CA** |

引脚表与此一致：CA 相关脚在 BGA116 上有焊球（例如 PA05 / XPI0_CA_D3 为球 H2）；CB 相关脚 PA08–PA15（XPI0_CB_D0/D1/D2/D3、SCLK、DQS、CS0、CS1）在 BGA116 和 100 eLQFP 列都是空的，只在 144 eLQFP 上引出。

HPM6200 系列只有 **1 个 XPI**，没有 XPI1。

因此片上 Flash 指南里“改接 CB 或 XPI1”这条路，在 **BGA116 上不存在**。144 脚封装才可以把外部 NOR 改到 CB，让片内 Flash 继续占用内部引脚组。

### 3. 和“从 CB 启动”的文章不是一回事

先楫技术文章《HPM6750 从 XPI0 CB 端口启动》说的是：OTP Word24 的 `XPI_PORT_SEL`（bit6）置 1，BootROM 改从 CB_CS0 探测 Flash，镜像 `nor_cfg_option` 也改成 CB（例如 `{0xfcf90002, 0x00000007, 0x00000100, 0x0}`），烧录工具 Connection 选 CB_CS0。

那是 **CB 焊球存在** 时的做法。HPM6284 的 BGA116 没有这些焊球，不能套用。

## 不能当成完全替换的原因

1. HPM6280 没有片内程序存储器，产品用法必须依赖外部 NOR。
2. HPM6284 的片内 Flash 走内部 PX，封装上的 CA 不再承担 XPI。
3. BGA116 除了 CA 没有第二套可外接 NOR 的 XPI 引脚。
4. 所以不存在“两颗芯片都把同一份程序烧进同一颗外部 NOR、上电都从它启动”的配置。

软件镜像都可以 XIP 在 `0x80000000`，但 NOR 选项字和烧录目标必须分开：6280 用外部第一组引脚，6284 用内部第二组引脚。

## 版本注意

片上 Flash 指南点名的是 **HPM6284Ixx1**。数据手册 Rev2.5 的命名里版本号可以是 x=1/2/3。换版本或换日期的手册时，要再对一次「封装引出功能差异」和片上 Flash 指南，确认 CA 失效和“BGA 仅 CA”没有改。

## 资料

- 《HPM6200 系列数据手册》Rev2.5（2025-01-14）：表 43 订购信息（6280 片上闪存为空，6284 为 4MB；XPI 数量为 1；IEP 为 7×7 BGA116）；表 44 封装引出功能差异；引脚复用表中的 XPI0_CA_* 与 XPI0_CB_*。
- 《HPM6000 系列 MCU 片上 Flash 使用指南》：SIP 的 PX / CA 限制，以及 HPM6284 的 CA 引脚表。
- 先楫 FAQ：`option[2]=0x00001000` 表示内部 Flash，`0x0` 表示外部 Flash。
- 先楫技术文章《HPM6750 从 XPI0 CB 端口启动》：仅适用于 CB 有引出的封装，不适用于 HPM6200 的 BGA116。
