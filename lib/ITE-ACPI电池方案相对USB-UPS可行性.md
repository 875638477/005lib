# ITE / ACPI 电池方案相对 USB-UPS（HC32F460 + USBIP）可行性

> 背景：已实现 HC32F460 USB 模拟 USBIP 的 USB-UPS 方案；痛点是 **USB 外设形态难以进入计算机电源策略内核路径**，且 **轮询/协议栈延迟导致实时性偏弱**。  
> 问题：能否改用笔记本常见的 **ITE EC（如 IT8987E）+ ACPI Control Method Battery**，让系统像笔记本一样管理电池？  
> 结论：**架构上完全可行，且更贴近 OS 原生电源管理；但前提是板级 LPC/eSPI + BIOS/ACPI 协同，不是“外挂一块 ITE 芯片”就能替代 USB 口。**

---

## 1. 两种路径本质差异

| 维度 | USB-UPS（现有） | ACPI 电池（笔记本路径） |
|------|-----------------|-------------------------|
| 总线位置 | USB 主机栈外设 | 芯片组 LPC / eSPI 上的 **Embedded Controller** |
| OS 认知 | HID UPS / 厂商驱动 / USBIP 远端设备 | `PNP0C09`（EC）+ `PNP0C0A`（Control Method Battery）+ `ACPI0003`（AC） |
| 数据路径 | USB 枚举 → 类驱动 → 用户态/电源服务 | EC 寄存器 / SMBus SBS → SCI/GPE → ACPI AML → 电池驱动 |
| 实时性 | ms～百 ms；受 USB 帧、USBIP、主机调度影响 | 通常 **亚 ms～数 ms** 级 SCI 通知；关键关机可走 EC/硬件路径 |
| 系统集成 | 难进 S3/S0ix、Critical Battery、厂商电源策略深处 | 任务栏电量、睡眠唤醒、低电强制关机、ACPI 电源策略原生支持 |
| 部署形态 | 外置/后装友好 | **必须进主板/定制机**（或改 BIOS 的整机） |

一句话：**USB-UPS 是“外设告知 OS 有电池”；ACPI 电池是“机器自己认为自己有电池”。**

---

## 2. IT8987E 在笔记本里实际做什么

IT8987E（ITE）是典型笔记本 **Embedded Controller**：

- 挂在 **LPC**（部分新平台为 eSPI 系列姊妹芯片）
- 提供 ACPI EC 接口（标准 I/O `0x62/0x66` 一类 PMC 通道）
- 键盘矩阵、风扇 PWM、ADC 监控、唤醒、部分电源时序
- 通过 **I2C/SMBus** 读智能电池（SBS：Smart Battery System）或自家电量计
- 固件把容量、电压、电流、状态写入 EC RAM；ASL 里 `_BIF/_BIX/_BST/_BTP` 等从 EC 字段映射出来
- AC 插拔、电池插入、低电等用 **EC SCI → GPE** 通知 OSPM

公开生态侧可参考：coreboot/Star Labs 等对 IT8987 的 `PNP0C09` + `battery.asl` 实现思路——**芯片只是载体，真正产品化的是 EC 固件 + ACPI 表 + 原理图电源轨。**

---

## 3. 对“能否用 ITE 做 ACPI 电池”的直接回答

### 3.1 可以，且能解决你提的两个痛点

1. **接入计算机系统内部**  
   OS 走标准 ACPI 电池栈，不依赖 USB 设备在不在、驱动装没装、USBIP 链路通不通。

2. **实时性更好**  
   - 状态变化：EC 主动 SCI，不必等 USB 轮询  
   - 临界低电：可在 EC 固件侧先做保护/关机请求，再等 OS  
   - 不经过 USB 主机控制器与（若用 USBIP）网络/虚拟化层

### 3.2 但必须同时满足四条，缺一不可

```
┌──────────────┐   LPC/eSPI    ┌─────────────┐  SMBus/I2C  ┌────────────┐
│ PCH / SoC    │◄─────────────►│ IT8987E EC  │◄───────────►│ 电量计/BMS │
│ + BIOS ACPI  │               │ + EC FW     │             │ + 电池包   │
└──────────────┘               └─────────────┘             └────────────┘
        │                              │
        │  DSDT/SSDT: EC + BAT0 + ADP1 │
        ▼                              ▼
   Windows/Linux 原生电池 UI / 电源策略   SCI/GPE 事件
```

1. **物理互连**：EC 必须接到平台的 LPC 或 eSPI，并具备合适的复位、时钟、STRAP、电源时序（常电轨）。  
2. **BIOS/UEFI**：开启/正确描述 EC；注入或编译进 **DSDT/SSDT**（`Device (EC)`、`Device (BAT0)`、`Device (ADP1)`、GPE 号、OperationRegion）。  
3. **EC 固件**：IT8987 的应用固件一般要 ITE 工具链/授权或深度逆向；不是烧个 Blink 就能出 ACPI 电池。  
4. **电源与电池侧**：充电、放电路径、AC Detect、Pack Present、安全保护仍要硬件 + BMS；EC 主要是 **策略与上报入口**，不是化学保护的唯一依赖。

> 若目标机是 **现成商用主板、仅 USB 可改**，则 **无法** 简单外挂 IT8987E 达到笔记本效果——没有 LPC 飞线进 PCH 并改 BIOS，ACPI 路径不成立。

---

## 4. 与现有 HC32F460 USB-UPS 如何取舍

### 4.1 适用场景建议

| 产品形态 | 更合适的方案 |
|----------|--------------|
| 外置 UPS、任意 PC 即插即用 | 继续 USB-UPS / HID UPS；可优化协议降低延迟 |
| 自研工控机/准系统/笔记本类整机 | **强烈建议 ACPI EC 电池路径**（ITE 或其它 EC） |
| 服务器/工作站改电，又要进 OS 电源策略 | 定制主板或改 BIOS 的 ACPI + EC/SBS；USB 只能作兼容层 |
| 已有主板但有调试 LPC 针脚 + 可改 BIOS | 可评估加 EC 子卡，工程量大 |

### 4.2 推荐的工程拆分（若做整机）

不必扔掉 HC32：

| 角色 | 芯片建议 | 职责 |
|------|----------|------|
| 功率/快环/保护 | HC32F460 / 电源 MCU（现有） | Buck/充电、OCP/OVP、电量算法、Pack 通信 |
| 系统侧 ACPI 门面 | IT8987E（或其它 EC） | LPC 上呈现标准 EC/Battery；SCI 上报 |
| 两者互联 | UART / I2C / SPI | 电源 MCU → EC：SOC%、电压、电流、状态、AC 在位 |

这样 **实时保护仍在电源 MCU**，**系统集成与 UI/策略在 EC+ACPI**，比“单靠 USB 上报”干净。

### 4.3 不一定非要 IT8987E

IT8987E 的价值是：笔记本量产验证多、ACPI EC 模型成熟。代价是：

- 资料与烧录工具链偏封闭  
- 对桌面定制项目，开源 **Chrome EC / Zephyr EC**、Nuvoton NPCX、部分可公开的 ITE IT8xxx 方案，有时更可控  
- 若平台只有 **PCH SMBus Host**，也可走 **非 EC 的 SBS 电池**（ACPI SMBus Operation Region），但 Windows 上兼容性与事件模型通常仍不如完整 EC 电池路径稳

选型原则：**优先保证“LPC/eSPI EC + 可维护固件 + 可改 ACPI”**，型号其次。

---

## 5. 实时性与“系统内部”再说明

### 5.1 为何 USBIP-UPS 实时性天然吃亏

- USB 中断/批量传输受主机调度  
- USBIP 再叠加网络或虚拟总线  
- 用户态守护进程/服务轮询常见  
- 设备拔掉或休眠策略变化时，电源视图容易“消失”

即便把 HID Report 周期收到 10 ms，也很难等价于 EC SCI + 固件侧临界处理。

### 5.2 ACPI 路径仍不是“硬件零延迟”

- AML 解释、OS 电池驱动仍有调度延迟  
- 真正的短路/过流必须在 **模拟前端 / 电源 MCU / 硬件保护**  
- ACPI 电池解决的是 **OS 可见性与策略协同**，不是替代硬件保护环

---

## 6. 落地检查清单（决定做不做 ITE）

**平台侧**

- [ ] 目标 SoC/PCH 是否引出 LPC 或 eSPI？电压域与复位是否可接 EC？  
- [ ] 是否具备改 BIOS 权限（AMI/Insydemore/coreboot/自研 UEFI）？  
- [ ] Windows / Linux 目标版本对 Control Method Battery 的验证计划？

**EC 侧**

- [ ] IT8987E（或替代料）供货、封装、最小系统原理图  
- [ ] EC 固件开发环境与签名/量产烧录  
- [ ] EC RAM map 与 ASL 字段约定（电压、电流、容量、状态位、AC）

**电池/电源侧**

- [ ] SBS 或自定义协议电量计；Pack Present / AC_OK 硬件脚  
- [ ] 与 HC32（若保留）的通信协议与看门狗  
- [ ] 低电关机：EC 通知 + 电源路径强制行为

**若以上多项为“否”**  
→ 继续打磨 USB-UPS（改 HID、缩短上报、避免 USBIP 热路径），或仅做“自研整机下一代”再上 ACPI。

---

## 7. 主板预留 TPM 接口能否用来和主板通信？

> 常见想法：主板已留 TPM 针座 → 是否可把 IT8987E / 电源 MCU 挂上去，走“系统内部总线”做 ACPI 电池？

### 7.1 短答

| 针座类型 | 总线 | 能否当 EC/电池通道用 |
|----------|------|----------------------|
| 老款 **LPC TPM**（TPM 1.2 时代常见） | LPC | **电气上可能摸到 LPC**，可做实验探针；**不能**当成笔记本 EC 接口直接产品化 |
| 新款 **SPI_TPM / TPM 2.0 header** | SPI（常与 BIOS SPI 域相关） | **不行**——协议、片选、固件模型都是 TPM，不是 ACPI EC |
| 板载 **fTPM / PTT**（无针座或针座禁用） | 片内 | 无可用外接总线 |

**结论：TPM 针座 ≠ EC 口。** 最多在部分老主板上提供一条 **残缺的 LPC 引出**；要做 ACPI 电池，仍然缺 EC 侧管理线、BIOS/ASL、以及与真 TPM 的冲突处理。

### 7.2 为什么“看起来像内部总线”却不够

桌面主板 TPM header 的本质是：**给 discrete TPM 模块用的厂商定制针座**，不是通用扩展总线。

1. **先分清针座世代**  
   - 手册写 `TPM`、`JTPM`、`TPMS` 且模块为 LPC 型 → 多半带 `LAD[3:0]` / `LFRAME#` 等 LPC 信号。  
   - 手册写 `SPI_TPM`、`TPM_SPI`，或配套模块是 SPI 小板 → **不是 LPC**，IT8987E（LPC EC）接不上。  
   - 华硕等存在 **外观相似、实为不同总线** 的 2.0 mm 针座，接错有损坏风险。

2. **即便是 LPC-TPM，引出也不等于完整 EC 互连**  
   典型 TPM 针座通常只保证 TPM 工作所需的一小撮脚（LPC 数据/帧、复位、时钟、3V/3VSB、有时 SERIRQ、少数 GPIO）。  
   笔记本 EC 还依赖或受益于：

   - SCI / SMI（或 eSPI Virtual Wire 等价物）做电源与电池事件  
   - 更完整的电源时序与常电域配合  
   - 与 Super I/O / EC 解码窗口、PIRQ 等平台资源规划  

   TPM header **很少按 EC 规格引出这些管理信号**。

3. **协议与固件模型不对**  
   - BIOS 对 TPM header 的期待是：**枚举/启用 Security Device（TPM）**，走 TCG 协议。  
   - ACPI 电池需要的是：`PNP0C09` EC + BAT/AC 对象 + GPE。  
   - 在 TPM 针座上挂一颗 IT8987，**不会**被 Windows 自动认成笔记本电池；没有改 ACPI 表就没有 Control Method Battery。

4. **资源与产品冲突**  
   - 占用 TPM 针座后，机器可能失去 discrete TPM（BitLocker / Win11 策略 / 行业合规）。  
   - 同 LPC 上若再挂非 TPM 从设备，需确认 PCH LPC decode、地址不冲突、复位时序可接受。  
   - 各品牌 pinout **不通用**，无法做“一块卡通吃所有预留 TPM 主板”的零售外设。

5. **新平台趋势更不利**  
   Intel 平台 LPC 逐步被 **eSPI** 替代；很多新板 TPM 已是 SPI，EC/SIO 在板内 eSPI 上且 **不引出 header**。此时预留 TPM **对做 EC 电池几乎无帮助**。

### 7.3 什么情况下“可以通信”——分层看

| 层级 | 通过 TPM 针座是否现实 |
|------|------------------------|
| A. 电气上看到 LPC 波形 / 做 Port 80 类调试 | 老款 LPC-TPM：**有先例**（调试卡插 TPM/LPC header） |
| B. 自定义从设备与主机做私有 LPC 寄存器交互 | 理论上可能，但要自研从设备 + 驱动/AML，工程量大 |
| C. 插上 IT8987，OS 出现标准电池图标与 ACPI 策略 | **否**（缺完整 EC 互连 + BIOS/ASL；针座也不是为 EC 设计） |
| D. 做成可售 UPS：任意预留 TPM 的主板即插即用 | **否**（pinout/总线世代分裂 + 必须改 BIOS） |

因此：若目标是 **“和主板通信”** 的广义调试/私有通道——LPC-TPM 在老平台上可以当作 **有限带宽的侧信道实验口**；若目标是 **笔记本同款 ACPI 电池**——TPM 针座 **不能替代** 原理图级 EC 设计。

### 7.4 若仍想利用预留针座，务实路径

1. **读手册确认**：`SPI_TPM` 直接放弃作 EC；仅评估明确的 LPC-TPM。  
2. **示波器/逻辑分析**：确认 LAD/LFRAME、时钟、3VSB 是否真实接到 PCH（有的针座未完整布线）。  
3. **先做 LPC 从设备 POC**（简单 I/O 窗口），不要一上来焊 IT8987 全功能 EC。  
4. **并行解决 BIOS**：能改 DSDT/SSDT 才谈得上电池对象；否则最多用户态私有协议。  
5. **产品上仍建议**：  
   - 通用 PC → USB-UPS  
   - 可控整机 → 原理图预留真正的 EC（LPC/eSPI），TPM 针座留给 TPM  

### 7.5 和本方案的关系（一句话）

**预留 TPM 接口：老板或许可“蹭到 LPC 边线”；新板多为 SPI TPM；都不能单靠插针座实现 IT8987 式 ACPI 电池。**

---

## 8. 结论

1. **可能，而且方向正确**：要用笔记本同款体验，就应走 **EC + ACPI 电池**，IT8987E 是其中一条成熟商业芯片路径。  
2. **不能指望“USB 换成 ITE 外设”**：没有板级 LPC/eSPI 与 BIOS ACPI，就没有真正的系统内电池。  
3. **也不能指望“TPM 针座当 EC 口”**：至多是老平台 LPC 实验入口；SPI_TPM 不可用；ACPI 电池仍要改 BIOS 与完整 EC 互连。  
4. **与现有 HC32 USB-UPS 关系**：短中期可并存——USB 服务通用 PC；ACPI EC 服务定制整机。最佳是 **HC32 管电，EC 管 ACPI 门面**。  
5. **实时性提升的边界**：系统策略与上报会明显优于 USB/USBIP；电池安全仍靠电源硬件与 MCU 快环。

---

## 9. 后续可展开的笔记（可选）

- 主流厂商 TPM / SPI_TPM / LPC_DEBUG 针座对照与识别方法  
- IT8987 最小系统与 LPC 接 PCH 的电源/复位时序  
- `_BIF/_BIX/_BST` 与 EC 共享内存字段表  
- HC32 ↔ EC 状态同步协议（含 AC/BAT 事件优先级）  
- USB-HID UPS 延迟优化清单（在不能改主板时的务实改进）
