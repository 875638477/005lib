---
name: cadence-schematic-layout
description: Preserve readable circuit topology while arranging Cadence Capture parts, grids, reference designators, values, net aliases, power symbols and off-page connectors. Use for reference-module integration or schematic cleanup with local real symbols and native save/reopen plus connectivity verification.
---

# Capture 原理图灵活设计、可读性与网格

先读端部对象优先规范[ENDPOINT-20261116.1](../../notes/capture_endpoint_symbols_standard.md)、[机器contract](../../notes/capture_endpoint_symbols_contract.json)及[用户正反例](../../notes/assets/endpoint_symbols_20261116/README.md)。真实OffPage/电源Name不可隐藏后用Alias代替；GND按批准原生profile保留完整接地图形。用[端部事实检查器](../../scripts/capture_layout_tools/check_endpoint_symbol_contract.py)核对其覆盖事实，独立补齐contract全部证据；盘点全部端部及接插件保留，缺事实为unknown，不得以facts checker、R/C或网表通过替代全量验收。

指定阻容来源：**000Resistor.OLB/R；002Capacitor.OLB/C；电解、钽用002Capacitor.OLB/C_ELEC**。这属于规格，其他本机同形符号也不能自行替代。

先读[唯一现行阻容规范](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)和[机器规则](../../notes/capture_rc_layout_contract.json)，报告规则版本、库同步身份和逐件核验字段。**必须项先执行，“灵活”仅在其范围内。** 交付须运行[检查器](../../scripts/capture_layout_tools/check_rc_layout_contract.py)并覆盖全部R/C；未知/缺项阻止“排版完成”。不能用网表或碰撞为零替代库一致、文字方向与引脚线对齐。

**主要目标是复用灵活判断方法**：先识别电路作用与阅读路径，分析参考为什么清楚，再比较横/竖/混合与连接方式，按局部冲突调整，最后独立验收可读性和电气。记录取舍理由；例图和坐标是上下文相关的结果，不能机械套用。详见[可复用判断流程](../../notes/capture_topology_preservation_and_grid.md#可复用的灵活判断方法)。

功能布局另读[用户参考与正反例](../../notes/capture_topology_preservation_and_grid.md)。当前用户约束优先；旧笔记中“一律横放”不再适用。

1. 固定最新源件、参考图、库、PinToPin/捕捉网格、可移动范围和SHA；冻结已有接插件位号清单，未用/额外/预留项也保留，未经用户对具体项明确许可不得删除。已授权的调位/增针/逐针修改单列白名单。实际工程的输入、脚本、日志、审核和交付在工程根，学习侧只放通用方法及获准示例。
2. 用户提供已优化原图时，直接复用其功能模块组织、横竖方向、引脚分组、直接导线、反馈/补偿/去耦及启停关系，按新需求局部融合。不可只抄BOM后把阻容拆成孤立网名行；不恢复已删除器件或参考中的已知电气错误。
3. 器件横/竖按功能灵活选择；阻容执行[RC-20261116.2](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)：全部使用本机正式库，不改图形/针距冒充；横件Ref/Value都横、竖件两者都竖；文字沿各自引脚线齐平、落网格、无干涉。确定器件方向后不得自由选择字向。先验证一横一竖的原生小样，保留物理脚号与极性。
4. 读取库针距、工程单位及实际热点。本用户参考1/2 of pin to pin，换工程重新换算；符号针距须与网格相容。修针距/原点时协调全部相关线和端口，不能逐对象盲目取整。
5. MOS使用真实G/S/D、沟道、体二极管及并脚/双管结构；BJT保留内部图形和箭头，不用空矩形代替离散器件。优先复用本机正确符号，逐脚核对封装。
6. [端部对象](../../notes/capture_endpoint_symbols_standard.md)先校准批准原生正例的库图形、实际热点、变换和Name显示框架，再按实际轮廓验方向：左端Name >>——电路，右端电路——<< Name；自身Name完整可见且在外侧，不能用线内Alias替代。电源保留合理母线短竖支路；GND完整可辨，是否显示Name依批准profile，若显示也须自身Name外置。保持原生类型、作用域及两端连接，完整图形/字形和邻接场景均要验，不能靠固定rotation/mirror猜。
7. 普通页内标识按[真实WireScalar/Alias规则](../../notes/capture_wire_net_alias_standard.md)；Alias的锚点在线规则不适用于端部Name。不能用注释、空白端口或悬空Name冒充连接，不能为增加标签凭空增加连接点。文字移动与电气热点移动分开核对。
8. 先保留清楚电路，后调整显示文字、端部符号，最后做确有需要的局部短线；不制造长U形回折、跨区穿表或文字挤压。依据[真实文字/原生变换](../../notes/capture_text_geometry_and_readability.md)建立完整模型，未知锚点列例外。
9. 在副本应用后原生保存、独立重开，逐Ref.Pin比较网络、NC、Value/装配/封装/关键属性；逐页检查完整元件、引脚、文字、电源/翻页、Alias、导线和表格。有PCB交接时核主工程新鲜网表和严格导入。显示、离线模型、GUI、PCB分别报告；网表等价不能替代可读性验收。
10. 用户要求后台时不启动或激活GUI。任一已知可见问题或关键显示对象漏验时，不把候选回写为“全页排版完成”。学习规则同步不能冒充实际工程整改完成。

[阻容细则](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)及[整体方法](../../notes/capture_compact_schematic_layout_method.md)的方向与对齐要求统一以RC-20261116.2为准。学习技能须后续读取复用，不等于模型权重训练或自动加载。
