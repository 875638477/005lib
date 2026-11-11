---
name: cadence-schematic-layout
description: Arrange Cadence Capture reference designators, values, net names and off-page connectors without text overlap or unnecessary wire detours. Use for schematic reconstruction or cleanup with existing local symbols, native save/reopen and connectivity verification.
---

# Capture 清晰排版与简洁连线

先读[完整方案](../../notes/capture_compact_schematic_layout_method.md)及[用户确认的同行示例与验收更正](../../notes/capture_rc_horizontal_ref_value_layout_spec.md#2026-11-11-用户确认的同行示例与排版验收)，再按当前工程范围实施。用户当前约束优先。

1. 确定工程源、原图、纸张、库路径和可移动范围；保存来源身份及副本。实际工程记录、计划和输出放工程根，学习库保存通用方法。
2. 优先用本地真实基本器件符号。普通二端阻容水平放置，新建或确需竖转横时默认 1 左/2 右；原本已横放者可保留 1 在右的脚序，不能只为交换左右位置而改线。位号紧挨左侧引线上方，Value 紧挨右侧引线上方，网络名排在位号和 Value 的外侧。依据实际字形避让，不固定上方两格，不改变规格或极性。
3. 先调整显示文字的偏移与对齐，再考虑必要的局部导线。网名、翻页符文字和引脚名/号保持独立可读，位号/Value 靠近所属引脚侧，不能挪远；阅读顺序为左网名 → 位号 → 按当前保留脚序绘制的器件 → Value → 右网名。以用户指定 R332/R333 的相对样式为参考，换符号重新测量，不硬编码示例偏移。
4. 已经清楚的器件与连接优先保留，不先挪走全部阻容再用全页寻路追回端点。保持短直连线和原图分组；独立信号就近接器件、测试点及端口。不要为了文字位置制造长 U 形折返或把无关信号汇成很长的线束。
5. 区分显示属性、网络别名、翻页符、层次端口与电源符号；移动文字不等于移动电气热点。同名文字不能代替跨页和层次连接校验。
6. 用原生引脚热点、旋转/镜像变换、实际字体范围及网格生成局部计划。候选位置限制在器件附近；未解决项显式记录，不遮盖、截短或无限缩小文字。
7. 在副本应用后原生保存、重开、重新导出。对实际交付源逐页检查显示，逐 RefDes.Pin 比较网络分区、名称、NC、属性和封装。有 PCB 交接要求时验证真实主工程网表及严格 Allegro 导入。
8. 分别报告原生重开、几何/排版、GUI 阅读、网表与 PCB 导入的证据和未验项。长回折、跨功能块穿表、文字重叠或关键显示对象漏验时，不能以“排版完成”回写正式版本；网表等价不能替代排版验收。不能用临时测试设计、旧日志或器件排除项冒充主工程通过。
9. 用户要求后台时，不为核查而启动或激活前台窗口。工程快照、脚本、临时文件、运行日志及验证结果全部放在用户指定的工程根；学习库只保存通用方法与示例。

阻容细则见[既有规范](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)；原生变换、GDI 文字边界及现有参数化工具见[几何与可读性](../../notes/capture_text_geometry_and_readability.md)。A3 等纸张要求由本次任务决定，不硬编码为所有工程的默认值。

此技能保存在用户 AI 学习库，供后续读取复用；保存文档不等于模型权重训练或新会话自动加载。

- 2026-11-11 全页码阻容规则：同功能分组、同组紧凑等行距；本体/位号/完整Value/两侧网名分别成列，电容组与电阻组间留白。全部旧页和新页适用；保留实际脚号与连接，不因示例图加回CS/nSLEEP偏置。详见[规则与图片](../../notes/capture_rc_horizontal_ref_value_layout_spec.md#2026-11-11-新增阻容按功能分组紧凑成行整齐成列所有页码)；不将规则同步写成批量改图验收。
