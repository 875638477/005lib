# CSD95480RWJ 同类智能功率级选型记录

- 记录日期：2026-08-23
- 基准型号：TI **CSD95480RWJ**
- 产品类别：Smart Power Stage（SPS，双 MOS + 驱动 IC 集成）
- 状态：TI 已标 **NRND**，不推荐新设计继续锁定

> 本文是选型备忘，不是 pin-to-pin 代换表。即使封装都是 5×6 mm，PWM 阈值、IMON 增益、REFIN、TAO/FLT、保护逻辑也可能不同，必须对照数据手册和前级多相控制器再核。

---

## 1. 基准规格（CSD95480RWJ）

| 项目 | 数值 |
|---|---|
| 类型 | NexFET Smart Power Stage（上管 + 下管 MOSFET + 驱动） |
| 连续 / 峰值电流 | 70 A / 90 A |
| VIN | 4.5–16 V（VDS 20 V） |
| 驱动 / 栅极供电 | VDD / PVDD = 4.5–5.5 V |
| VOUT | 最高约 5.5 V |
| 开关频率 | 最高 1.25 MHz |
| 封装 | 5×6 mm，41-pin VQFN-CLIP（RWJ） |
| PWM | 三态，3.3 V / 5 V 兼容 |
| 轻载 | FCCM / 二极管仿真 |
| 遥测 | 温度补偿双向电流检测 + 模拟温度输出（TAO） |
| 其它 | 集成 bootstrap、故障监测、优化死区 |
| 典型场景 | 高功率密度同步 buck、CPU/GPU/服务器 VRM |

资料：

- 产品页：<https://www.ti.com/product/CSD95480RWJ>
- 数据手册：<https://www.ti.com/lit/ds/symlink/csd95480rwj.pdf>

---

## 2. 选型结论（先看这三档）

| 场景 | 推荐 | 理由 |
|---|---|---|
| 尽量少改板，继续用 TI 控制器（TPS536xx 一类） | **CSD95410RRB** | TI 现役主力，同 5×6 业界通用封装，电流/频率更好 |
| 规格对齐 70 A SPS，品牌可换 | **TDA21472 / TDA21490 / AOZ52177QI** | 都是双 MOS + 驱动 + IMON/TMON |
| 新项目、电流密度更高 | 直接上 **90 A 档** | 不要再锁 70 A NRND 料 |
| 只要驱动 + 双 MOS，不要电流遥测 | 走普通 DrMOS | 不能当 95480 的功能对等件 |

---

## 3. TI 同系列（优先）

| 型号 | 电流 | 封装 | 状态 | 备注 |
|---|---|---|---|---|
| **CSD95410RRB** | 90 A peak continuous | 5×6 mm，41-pin VQFN-CLIP | Active | 新设计首选；频率至 1.75 MHz；30 A 时效率 >95% |
| **CSD95411RRB** | 65 A peak continuous | 同封装、同接口风格 | Active | 电流档略低 |
| CSD95490Q5MC | 75 A / 105 A peak | 5×6 DualCool | NRND | 顶冷更好，但同样不推荐新设计 |

CSD95410 产品页：<https://www.ti.com/product/CSD95410RRB>

---

## 4. 跨品牌同类 SPS（双 MOS + 驱动 + IMON/TMON）

这类才是 CSD95480 的真正同类，不是普通 DrMOS。

| 型号 | 厂商 | 电流 | 封装 | 说明 |
|---|---|---|---|---|
| **TDA21472** | Infineon | 70 A | 5×6 PQFN | 规格最接近：5 mV/A 电流检测、8 mV/°C 温度、至 1.5 MHz |
| **TDA21490** | Infineon | 70 A DC / 90 A peak | 5×6 | 电流档对齐 95480，效率/保护更完整 |
| **TDA21570** | Infineon | 70 A | 5×6 | 服务器 / GPU VRM 常见 |
| **AOZ52177QI** | AOS | 70 A | 5×6 级 | 带 5 mV/A IMON，可给多相控制器做均流 |
| **MP86957** | MPS | 约 70 A | 单片半桥 | 频率可到 3 MHz，适合高密度 CPU/GPU |

资料：

- TDA21472：<https://www.infineon.com/part/TDA21472>
- TDA21490：<https://www.infineon.com/part/TDA21490>
- AOZ52177QI：AOS DrMOS / Smart Power Stage 系列

---

## 5. 普通 DrMOS（无模拟电流遥测）

接口更简单，**不能当 95480 的功能对等件**，只能用在不需要 IMON/TMON 的场合。

| 型号 | 厂商 | 电流 | 封装 | 备注 |
|---|---|---|---|---|
| SiC631 | Vishay | 约 50 A | 5×5 MLP55-31L | 普通 DrMOS |
| SiC645 / SiC645A | Vishay | 60 A | MLP55-32L | 带电流/温度监测，更接近 SPS |
| SiC789 / SiC789A | Vishay | 60 A | 6×6 MLP66-40L | 普通 DrMOS，3.3 V / 5 V PWM 分型号 |
| AOZ5116QI-08 | AOS | 55 A 连续 / 120 A 峰值 | 5×5 QFN-31L | VIN 可到 25 V，频率至 2 MHz |
| MP86956 | MPS | 70 A | 单片半桥 | 100 kHz–3 MHz |

---

## 6. 国产可评估（供货 / 成本）

| 型号 | 厂商 | 电流 | 说明 |
|---|---|---|---|
| **PN7864** | 芯朋微 | 70 A | 5–16 V，带电流/温度感测 |
| **PN7865 / PN7866** | 芯朋微 | 90 A | 同系列升高一档 |
| **JWH7067 / JWH7079** | 杰华特 | 70 A / 90 A | 服务器、GPU 供电已有量产案例；JWH7079 为 4×6 mm |
| **SQ29670** | 矽力杰 | 70 A | 标准封装 SPS，IMON 用 AutoZero |
| **MK684x / MK6850 / MK6851** | 茂睿芯 | 至 90 A | 4×6 / 5×6 |

国产料要重点核：PWM 三态窗口、IMON 增益（常见 5 mV/A 或 5 µA/A）、REFIN 范围、和现有多相控制器是否匹配。

---

## 7. 换料核对清单

换料或新选时至少对这几项：

1. PWM 高 / 三态 / 低阈值，以及 3.3 V / 5 V 逻辑是否匹配
2. IMON 增益、极性、带宽，以及 REFIN 范围
3. TAO 斜率、FLT 极性、EN / FCCM 极性
4. OCP / OTP / UVLO / 相故障行为
5. VIN 耐压、开关节点尖峰、bootstrap 是否自动补电
6. 封装焊盘是否真是同一套 5×6 industry common footprint
7. 前级多相控制器兼容性（TI TPS536xx、Infineon XDPE、Renesas ISL、MPS MP29xx 等）

---

## 8. 待补充（下次选型时补齐）

- [ ] 应用：CPU VRM / GPU / 服务器 / POL
- [ ] 输入电压、输出电压、每相电流、开关频率
- [ ] 使用的多相控制器型号
- [ ] 是否必须要 IMON / TMON
- [ ] 是否要求 pin 兼容或只要求功能同类
- [ ] 供货渠道、目标成本、是否必须国产

---

## 9. 修订

| 日期 | 说明 |
|---|---|
| 2026-08-23 | 初稿：以 CSD95480RWJ 为基准整理同类 SPS / DrMOS / 国产料 |
