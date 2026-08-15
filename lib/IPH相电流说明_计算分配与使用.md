# IPH 是什么：相电流的计算、分配与使用

> 适用：12V **四相交错** Buck-Boost（以及两相时的 IphA/IphB）  
> 相关 IO：`IPH1…IPH4` → ADC0 / PC08–PC11（见 IO 分配表）

---

## 1. IPH 是什么？

**IPH = Phase Current（相电流）**，指 **每一相交错功率级自己的电感电流**（或该相半桥支路电流），不是整路输出总电流。

| 符号 | 含义 | 典型量级（本方案） |
|------|------|-------------------|
| **IPH1…IPH4** | 第 1～4 相电感电流 \(I_{L1}…I_{L4}\) | 满载均分约 **25 A/相**（100 A ÷ 4） |
| **IOUT1** | 12V 输出总线总电流 | 满载 **100 A** |
| **IIND2 / IIND3** | 5V / 3.3V **单相**电感电流 | 满载约 30 A（该路只有一相，≈ IOUT） |

关系（理想均流、忽略损耗）：

\[
I_{\mathrm{OUT1}} \approx I_{\mathrm{PH1}} + I_{\mathrm{PH2}} + I_{\mathrm{PH3}} + I_{\mathrm{PH4}}
\]

```
Vin ──┬──[半桥1]── L1 ──┬── 传感 IPH1 ──┐
      ├──[半桥2]── L2 ──┼── 传感 IPH2 ──┤
      ├──[半桥3]── L3 ──┼── 传感 IPH3 ──┼──► Vout 12V ── 传感 IOUT1 ──► 负载
      └──[半桥4]── L4 ──┴── 传感 IPH4 ──┘
```

- **IPH**：管「每一相自己转多少」→ 均流、逐相限流、电流环  
- **IOUT**：管「总共输出多少」→ CC 模式、总过流、功率计量  

---

## 2. 如何计算？

### 2.1 每相平均电流（设计目标）

\[
I_{\mathrm{PH,avg}} = \frac{I_{\mathrm{OUT}}}{N}
\]

本方案 \(N=4\)，\(I_{\mathrm{OUT,max}}=100\,\mathrm{A}\)：

\[
I_{\mathrm{PH,avg,max}} = 25\,\mathrm{A}
\]

再留裕量（均流误差、瞬态、纹波峰值），**传感满量程建议按 35～40 A/相** 设计。

### 2.2 电感电流纹波（选型用）

四开关 Buck-Boost 在不同 Vin 下公式不同，粗算可用（Buck 区示例）：

\[
\Delta I_L \approx \frac{(V_{\mathrm{in}}-V_{\mathrm{out}})\cdot D}{L\cdot f_{sw}}
\]

Boost 区用对应伏秒。经验目标：

\[
\Delta I_L \approx (20\%\sim40\%)\cdot I_{\mathrm{PH,avg}}
\]

即约 **5～10 A 峰峰值** @ 25 A 平均。  
峰值相电流：

\[
I_{\mathrm{PH,peak}} \approx I_{\mathrm{PH,avg}} + \frac{\Delta I_L}{2}
\]

**OCP / ADC 满量程要盖住 \(I_{\mathrm{PH,peak}}\) 再加 20%～30%。**

### 2.3 分流电阻 + INA240 + ADC 换算

**分流压降：**

\[
V_{\mathrm{shunt}} = I_{\mathrm{PH}} \cdot R_{\mathrm{sense}}
\]

**INA240 输出：**

\[
V_{\mathrm{adc}} = V_{\mathrm{shunt}} \cdot G_{\mathrm{INA}} + V_{\mathrm{ref}}
\]

（双向测量时有参考电压 \(V_{\mathrm{ref}}\)，常见 Vcc/2。）

**软件还原电流：**

\[
I_{\mathrm{PH}} = \frac{V_{\mathrm{adc}} - V_{\mathrm{ref}}}{G_{\mathrm{INA}}\cdot R_{\mathrm{sense}}}
\]

#### 推荐算例（单相 IPH，满量程 ~40 A）

| 参数 | 取值 | 说明 |
|------|------|------|
| \(R_{\mathrm{sense}}\) | **1 mΩ** | \(P=I^2R\)：25 A 时 0.625 W；40 A 时 1.6 W → 选 ≥3 W 分流 |
| 满电流压降 | 40 A × 1 mΩ = **40 mV** | 落在 INA229 ±40.96 mV 档也可共用 |
| INA240 增益 \(G\) | **20 V/V** | 40 mV × 20 = **0.8 V** 摆幅 |
| 参考 | 1.65 V（3.3V/2） | 双向：0 A → 1.65 V；+40 A → 2.45 V；−40 A → 0.85 V |
| ADC | 16bit，参考 3.3 V | 约 50 µV/LSB → 电流分辨约 **2.5 mA/LSB**（理想） |

**IOUT1（100 A）** 另用更大电流分流，例如 **0.25～0.5 mΩ**，增益另选，使满载输出仍在 ADC 量程内。

**功耗提醒：**

\[
P_{\mathrm{sense}} = I^2 R_{\mathrm{sense}}
\]

四相各 1 mΩ @ 25 A → 每相 0.625 W，总共 2.5 W；IOUT 若 0.25 mΩ @ 100 A → 2.5 W。注意散热与开尔文取样。

### 2.4 与 INA229 共用分流时

同一 \(R_{\mathrm{sense}}\) 可同时接 INA240（快环）和 INA229（慢环），但：

- 星形 Kelvin  
- 各自滤波  
- \(V_{\mathrm{shunt,max}}\) 同时满足 INA229 量程（±40.96 或 ±163.84 mV）

---

## 3. 如何分配？

### 3.1 硬件：一相一个 IPH 传感器

| 相 | 信号名 | 取样位置（推荐） | ADC 脚（IPA2） |
|----|--------|------------------|----------------|
| Phase1 | IPH1 | **L1 上**（或该相低端分流） | PC08 |
| Phase2 | IPH2 | L2 | PC09 |
| Phase3 | IPH3 | L3 | PC10 |
| Phase4 | IPH4 | L4 | PC11 |
| 总线 | IOUT1 | **输出汇流后** | PC12 |

**推荐取样点：电感电流**（串联在 Lx 回路），因为：

- 连续反映该相能量存储与平均电流  
- 适合平均电流模式 / 均流  
- 比只采开关管脉冲电流更稳  

也可低端分流（源极到功率地），布局简单，但注意地弹与共模。

### 3.2 不要用「一个 IOUT 代替四个 IPH」

| 只用 IOUT | 有 IPH1…4 |
|-----------|-----------|
| 无法知道哪一相过载 | 可逐相限流 |
| 均流只能开环估 | 可闭环均流 |
| 一相失效不易发现 | 可侦测相电流失衡 |

四相 **必须** 每相一个 IPH；IOUT 建议保留（CC、总保护、与 ΣIPH 交叉校验）。

### 3.3 5V / 3.3V 为什么叫 IIND 不是 IPH？

单相没有「多相」概念，电感电流只有一路，表里写成 **IIND2 / IIND3**，用法等于「该路的 IPH」，同时近似等于该路 IOUT（再加输出电容电流）。

---

## 4. 如何使用？（控制与保护）

### 4.1 采样时刻

用 PWM TRGM 在 **电感电流谷点或峰值** 触发 ADC（与该相 PWM 同步）：

| 模式 | 采样点 | 用途 |
|------|--------|------|
| 峰值电流模式 | 开通过程中的电流峰 | 逐周期限流 |
| 平均电流模式 | 谷点或中点 | 均流、电流环（更常用数字实现） |

四相相移 90°，**各相在自己的 PWM 周期里触发自己的 IPH 通道**，不要四相共用一个固定时刻采四路（会采到不同电相位，均流算歪）。

### 4.2 电流环（每相内环）

对每一相：

1. 电压环（或 CC 给定）产出 **总电流指令 \(I_{\mathrm{ref,total}}\)**  
2. 均分：\(I_{\mathrm{ref,k}} = I_{\mathrm{ref,total}} / 4 + \Delta I_{\mathrm{share},k}\)  
3. 电流环：\(I_{\mathrm{ref,k}}\) 与 **IPHk** 做 PI → 该相占空比  

### 4.3 均流环

\[
I_{\mathrm{avg}} = \frac{1}{4}\sum_{k=1}^{4} I_{\mathrm{PHk}}
\]

\[
\Delta I_{\mathrm{share},k} = G_{\mathrm{share}}\cdot (I_{\mathrm{avg}} - I_{\mathrm{PHk}})
\]

把 \(\Delta I_{\mathrm{share},k}\) 叠到该相电流给定，消除布局/电感公差造成的偏流。  
均流环带宽应 **低于** 电流环（例如电流环 \(f_{sw}/10\)，均流再慢 5～10 倍）。

### 4.4 保护

| 层级 | 用谁 | 动作 |
|------|------|------|
| 逐周期 / 硬件快 | **IPHk** → ACMP 或数字比较 | 超阈值立刻收窄该相占空比或 Fault |
| 总输出过流 | **IOUT1** | 降额 / 关机 |
| 相失衡 | max(IPH)−min(IPH) 过大 | 告警或切相 |
| 计量 / 慢保护 | INA229（可与 IPH 同分流） | 上报、锁存 |

### 4.5 与 CV / CC 的关系

- **CV**：电压环出 \(I_{\mathrm{ref,total}}\)，再经均分 + IPH 电流环  
- **CC**：\(I_{\mathrm{ref,total}}\) 直接等于设定电流（如 80 A），同样均分到四相 IPH  
- 显示/上位机电流：优先用 **IOUT1 或 INA229**；IPH 主要用于控制  

### 4.6 软件伪代码（概念）

```c
/* 每相在各自 ADC 完成中断或统一控制节拍中 */
Iph[k] = adc_to_ampere(adc_iph[k]);   /* 按 Rsense、Gain、Vref 换算 */

I_avg = (Iph[0]+Iph[1]+Iph[2]+Iph[3]) * 0.25f;
I_ref_total = voltage_loop(Vout_ref, Vout_meas);  /* 或 CC 给定 */

for (k = 0; k < 4; k++) {
    I_share = Kshare * (I_avg - Iph[k]);
    I_ref_k = I_ref_total * 0.25f + I_share;
    duty[k] = current_loop(I_ref_k, Iph[k]);     /* PI */
    if (Iph[k] > I_ocp_ph) pwm_fault_phase(k);   /* 逐相 OCP */
}
pwm_update_all(duty);
```

---

## 5. 一句话对照

| 问题 | 答案 |
|------|------|
| IPH 是什么？ | **每一相的电感（支路）电流** |
| 怎么算？ | \(I_{PH}\approx I_{OUT}/4\)；传感按 35～40 A/相满量程；\(I=V_{\mathrm{shunt}}/(R_s G)\) |
| 怎么分配？ | **一相一个分流+INA240** → IPH1…4；另加 IOUT1 |
| 怎么用？ | 电流环 + 均流 + 逐相 OCP；IOUT/INA229 做总电流与计量 |

---

## 6. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-08-15 | 初版：IPH 定义、计算、分配、控制使用 |
