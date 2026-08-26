# RK3576 数字麦克风与多麦语音识别记录

- 记录日期：2026-08-26
- 结论摘要：**可以用数字麦做双麦、四麦语音识别**；识别速度由模型决定，麦数几乎不影响推理耗时。

---

## 1. 结论

| 问题 | 结论 |
|------|------|
| 能否接数字麦克风 | 能。优先走片内 **PDM0 / PDM1** |
| 双麦语音识别 | 能，引脚最少、最容易落地 |
| 四麦语音识别 | 能。单路 PDM 控制器即可带 4 路，理论上最多 8 路 |
| 识别速度 | 命令词约 **100–300 ms**；流式 ASR 约 **0.3–1 s**；Whisper 约 **0.6–3.4 s** |

---

## 2. 芯片音频能力（硬件）

来源：Rockchip RK3576 Datasheet V1.6、Brief Datasheet。

### 2.1 与数字麦直接相关

| 模块 | 规格 |
|------|------|
| PDM0 / PDM1 | 各最多 **8 通道**，16–24 bit，采样率最高 192 kHz |
| 工作模式 | **仅 PDM Master 接收**（时钟由 SoC 输出） |
| 其他 | 支持增益控制 |
| SAI | 5 路 SAI，I2S / TDM / PCM；SAI0/1 为 4TX/4RX，SAI2/3/4 为 1TX/1RX |
| 片内 Codec | 2 通道数字 **DAC**（无模拟 MIC ADC） |
| ASRC | 双 2 通道 + 双 4 通道 |
| NPU | 双核，**6 TOPS@INT8**（稀疏加速标注），INT4/8/16、FP16、BF16、TF32 |
| CPU | 4× Cortex-A72 + 4× Cortex-A53 |

要点：RK3576 **没有片内模拟 MIC ADC**，数字麦（PDM / I2S）是正路。

语音采集常用参数：**16 kHz / 16 bit**。驱动层常见可用范围约 8–48 kHz，并带高通滤波（3.79 / 60 / 243 / 493 Hz）。

### 2.2 数字麦类型

| 类型 | 接口 | RK3576 接法 |
|------|------|-------------|
| PDM MEMS（最常见） | CLK + DATA | 直接挂 PDM0 或 PDM1 |
| I2S MEMS | BCLK + LRCK + DATA | 挂 SAI（I2S/TDM） |
| 外置 ADC / Codec | I2S / TDM | 挂 SAI |

---

## 3. 接线与声卡

### 3.1 双麦

- 1 根共享 `PDM_CLK` + 1～2 根 `PDM_SDI`
- 不少 PDM 麦支持左右声道（时钟沿分 L/R），**一根数据线即可出立体声**

### 3.2 四麦

- 1 根共享 `PDM_CLK` + **4 根** `PDM_SDI`
- 信号流：

```
数字麦 → PDM 控制器（硬件解调为 PCM）→ DMA → 内存 → 预处理 / ASR
```

### 3.3 软件侧

- 控制器兼容：`rockchip,rk3576-pdm`
- 数字麦无独立 codec，用 `rockchip,dummy-codec` 虚拟编解码器
- 声卡：`simple-audio-card`，名称常见为 `rockchip,pdm-mic-array`
- 四麦 pinctrl 需拉齐 `SDI0–SDI3` 与 CLK

**板级注意：** PDM 引脚与 LCD、网口、eMMC 等复用，原理图必须先确认 CLK / SDI 空闲。

---

## 4. 双麦 vs 四麦

麦数不决定“能不能识别”，决定的是远场、抗噪和指向性。

| 方案 | 算法 | 适用 |
|------|------|------|
| 双麦 | AEC + 简单波束 / 差分降噪 | 近讲、中场、带喇叭回声消除 |
| 四麦 | BF + AEC + ANR + AGC | 远场、会议室、智能音箱、车载 |

瑞芯微现成库：**RKAP / rkaudio**（AEC、BF、ANR、AGC、高通）。四麦需按阵列几何调参（间距、麦序、回采通道）。

识别一般吃**处理后的单路增强语音**，不是 4 路同时进 ASR。因此：

- 四麦比双麦更准、更远、更抗噪
- 四麦几乎不增加识别推理时间（多的是约 10–30 ms 预处理）
- RK3576 CPU 跑双麦 / 四麦预处理实时足够

有扬声器时，AEC 需要回采（loopback）。

---

## 5. 识别速度（公开实测）

采集是硬件 PDM，延迟毫秒级，可忽略。耗时在预处理和模型。

### 5.1 命令词 / 唤醒

- NPU 跑小模型
- 说完后大约 **100–300 ms** 出结果

### 5.2 流式 / 离线 ASR

| 模型 / 场景 | 音频 | RK3576 耗时 | RTF | 来源 |
|-------------|------|-------------|-----|------|
| SenseVoiceSmall | 15 s 日语 | 约 **1.87 s** | **0.13**（约 7 倍实时） | 三信电气 2025-09 |
| 流式中间结果 | — | 约 **400 ms** 刷新 | 可边说边出字 | LANGO / SenseVoice RKNN |
| Whisper 短指令 | Home Assistant 典型短句 | 约 **0.626 s** | — | Seeed reComputer AI Lab |
| Whisper | 5 s 英语 | 约 **3.4 s** | **0.68** | 三信电气 2025-09 |

同批对比（仅作参照）：

| 模型 | RK3566 | RK3576 | RK3588 |
|------|--------|--------|--------|
| Whisper（5 s 英语） | 9.7 s（RTF 1.9） | 3.4 s（0.68） | 1.97 s（0.39） |
| SenseVoiceSmall（15 s 日语） | 7 s（0.47） | 1.87 s（0.13） | 1.15 s（0.08） |

1～3 秒短指令，SenseVoice / Paraformer 说完后再等大约 **0.3–0.8 s** 出最终文本。

### 5.3 端到端语音助手（Seeed，RK3576 NPU）

| 阶段 | 时间 |
|------|------|
| Whisper 转写 | 0.626 s |
| LLM 回复 | 2.82 s |
| Piper TTS | 0.474 s |

识别通常不是端到端最长的一环。

### 5.4 和麦数的关系

双麦改四麦：**识别推理时间基本不变**，增加的是波束成形 / AEC 预处理。

---

## 6. 选型建议

| 场景 | 建议 |
|------|------|
| 命令词 / 近讲 | 双麦 PDM，体感约 200 ms 内 |
| 远场、有喇叭、要抗噪 | 四麦 PDM + RKAP（AEC+BF），识别速度几乎不变 |
| 整句转写 / 多语种 | SenseVoice RKNN，短句约 0.3–1 s |
| 高精度离线转写 | Whisper，短句约 0.6–3 s，不适合强实时 |

---

## 7. 软件落地要点

1. Device Tree：启用 `pdm0` 或 `pdm1`，绑 `dummy-codec`，注册 `pdm-mic-array`。
2. 验证采集：`tinycap` / `arecord` 录 2ch 或 4ch，确认通道不串、无时钟毛刺。
3. 预处理：接入 RKAP（AEC/BF/ANR/AGC）；有喇叭必须配回采。
4. 识别：命令词用小模型；整句用 SenseVoice / Paraformer RKNN；不要默认上大号 Whisper。
5. 调参：四麦阵列间距、麦序、回采通道位置必须与硬件一致。

---

## 8. 资料来源

- Rockchip RK3576 Datasheet V1.6：https://rockchip.fr/RK3576%20datasheet%20V1.6.pdf
- Rockchip RK3576 Brief Datasheet：https://www.rock-chips.com/uploads/pdf/2024.3.18/192/RK3576%20Brief%20Datasheet.pdf
- RK3576 Android14 MIC 调试（PDM 阵列接线与 dummy-codec）
- 三信电气：Rockchip NPU 文字起こし性能検証（2025-09-26）
- Seeed：在 RK3576 上构建私有 Home Assistant 语音助手
- Radxa Docs：Whisper on Rockchip NPU
- rkvoice-stream / OpenVoiceStream（RK3576/RK3588 端侧 ASR）
- Rockchip Microphone Array Tuning / RKAP（AEC、BF、ANR）

---

## 9. 待补充（场景未定时）

若后续明确以下信息，可把方案收敛到麦型、阵列间距和模型：

- 近讲还是远场
- 是否有喇叭（要不要 AEC）
- 命令词还是整句转写
- 系统是 Linux 还是 Android
