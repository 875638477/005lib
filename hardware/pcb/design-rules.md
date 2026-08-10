# PCB 设计规则摘要

## 网络优先级

1. `VBUS_12V` / `V12A` / `V12B` / `PGND` — 最高载流
2. `SB_*` / `SBS_BUS` — 中等，注意反灌二极管极性
3. `SCL`/`SDA`/`EN#`/`PRE`/`PSOK` — 信号完整性与防护

## 清表规则（建议导入 CAD）

```text
clearance_signal = 0.2mm
clearance_power  = 1.0mm
track_signal     = 0.25mm
via_signal       = 0.3mm drill / 0.6mm pad
via_power        = 0.45mm drill / 0.85mm pad, via farm
min_ring         = 0.15mm
```

## 覆铜

- L2 全部 `PGND`
- L3 全部 `VBUS_12V`（若层任务如此分配）
- Top/Bottom 功率区与内层通过 via farm 缝合
- 信号区局部挖空避免切割回流

## 测试点

至少引出：`VBUS_12V`, `PGND`, `SB_A`, `SB_B`, `EN_A#`, `EN_B#`, `PRE_A`, `PRE_B`, `SCL`, `SDA`, `3V3`

## 丝印

- 标明 SLOT A / SLOT B 插入方向箭头
- 标明铜柱极性
- 标明 ADR 跳线含义
- 标注“MAX CURRENT / SAFETY WARNING”
