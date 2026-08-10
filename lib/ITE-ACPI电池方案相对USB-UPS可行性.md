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

## 7. 结论

1. **可能，而且方向正确**：要用笔记本同款体验，就应走 **EC + ACPI 电池**，IT8987E 是其中一条成熟商业芯片路径。  
2. **不能指望“USB 换成 ITE 外设”**：没有板级 LPC/eSPI 与 BIOS ACPI，就没有真正的系统内电池。  
3. **与现有 HC32 USB-UPS 关系**：短中期可并存——USB 服务通用 PC；ACPI EC 服务定制整机。最佳是 **HC32 管电，EC 管 ACPI 门面**。  
4. **实时性提升的边界**：系统策略与上报会明显优于 USB/USBIP；电池安全仍靠电源硬件与 MCU 快环。

---

## 8. 后续可展开的笔记（可选）

- IT8987 最小系统与 LPC 接 PCH 的电源/复位时序  
- `_BIF/_BIX/_BST` 与 EC 共享内存字段表  
- HC32 ↔ EC 状态同步协议（含 AC/BAT 事件优先级）  
- USB-HID UPS 延迟优化清单（在不能改主板时的务实改进）
