---
name: cadence-schematic-layout
description: Preserve readable circuit topology while arranging Cadence Capture parts, grids, reference designators, values, net aliases, power symbols and off-page connectors. Use for reference-module integration or schematic cleanup with local real symbols and native save/reopen plus connectivity verification.
---

# Capture 原理图灵活设计、可读性与网格

**任务开始优先：功能模块紧凑与文字同行**。先读[MODULETEXT-20261117.1](../../notes/capture_compact_modules_and_single_line_text.md)。按电路功能保留清楚拓扑，局部选择横/竖/混合，收紧组内留白及过长引线；不能用扩大纸张代替。盘点全部可见文字，短通道名等既核实际控制字符，也核原生文字框自动折行；有意多行逐对象说明。完成这些优先检查后再执行下面各层规则，不能仅同步笔记。

**2026-11-17竖排贴线纠正优先**：用户指定DP_C5水平样式OK、DP_C8竖排文字离引脚线太远。先读[实际字形贴线规则](../../notes/capture_rc_vertical_proximity_20261117.md)。竖件Ref/Value除了都竖、落网格，还须实际字形紧挨自己的引脚线且无干涉；锚点对齐不构成间距通过。先以当前库/字体的真实原生文字框架校准一横一竖，绑定正例间距上限，再全量执行；不得机械把水平偏移旋转、从截图像素猜坐标或只保存规则而不改当前实例。

**最高优先三项**：①GND从电路向外最大bar靠内、其余按真实库数量向外逐级缩短，左右镜像；电源Name紧贴真实符号外缘，不能只有外置没有间距上限。②阻容按功能紧凑成组，收紧行列间隔。③器件/接插件到网络符号用适当短引线，不能因满页表格拉成长线。先读[紧凑度与引线标准COMPACT-20261116.1](../../notes/capture_schematic_compactness_standard.md)、[contract](../../notes/capture_schematic_compactness_contract.json)，并运行[独立紧凑度检查器](../../scripts/capture_layout_tools/check_schematic_compactness.py)。保留原理拓扑、库、完整文字与无干涉是收紧的前提；缺几何/候选证据为unknown。旧RC五项或端部检查通过均不能替代新增紧凑度验收。


先读端部对象优先规范[ENDPOINT-20261116.2](../../notes/capture_endpoint_symbols_standard.md)、[机器contract](../../notes/capture_endpoint_symbols_contract.json)及[用户正反例](../../notes/assets/endpoint_symbols_20261116/README.md)。真实OffPage/电源Name不可隐藏后用Alias代替；GND按批准原生profile保留完整接地图形。用[端部事实检查器](../../scripts/capture_layout_tools/check_endpoint_symbol_contract.py)核对其覆盖事实，独立补齐contract全部证据；盘点全部端部及接插件保留，缺事实为unknown，不得以facts checker、R/C或网表通过替代全量验收。

指定阻容来源：**000Resistor.OLB/R；002Capacitor.OLB/C；电解、钽用002Capacitor.OLB/C_ELEC**。这属于规格，其他本机同形符号也不能自行替代。

先读[唯一现行阻容规范](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)和[机器规则](../../notes/capture_rc_layout_contract.json)，报告规则版本、库同步身份和逐件核验字段。**必须项先执行，“灵活”仅在其范围内。** 交付须运行[检查器](../../scripts/capture_layout_tools/check_rc_layout_contract.py)并覆盖全部R/C；未知/缺项阻止“排版完成”。不能用网表或碰撞为零替代库一致、文字方向与引脚线对齐。

**主要目标是复用灵活判断方法**：先识别电路作用与阅读路径，分析参考为什么清楚，再比较横/竖/混合与连接方式，按局部冲突调整，最后独立验收可读性和电气。记录取舍理由；例图和坐标是上下文相关的结果，不能机械套用。详见[可复用判断流程](../../notes/capture_topology_preservation_and_grid.md#可复用的灵活判断方法)。

功能布局另读[用户参考与正反例](../../notes/capture_topology_preservation_and_grid.md)。当前用户约束优先；旧笔记中“一律横放”不再适用。

1. 固定最新源件、参考图、库、PinToPin/捕捉网格、可移动范围和SHA；冻结已有接插件位号清单，未用/额外/预留项也保留，未经用户对具体项明确许可不得删除。已授权的调位/增针/逐针修改单列白名单。实际工程的输入、脚本、日志、审核和交付在工程根，学习侧只放通用方法及获准示例。
2. 用户提供已优化原图时，直接复用其功能模块组织、横竖方向、引脚分组、直接导线、反馈/补偿/去耦及启停关系，按新需求局部融合。不可只抄BOM后把阻容拆成孤立网名行；不恢复已删除器件或参考中的已知电气错误。
3. 器件横/竖按功能灵活选择；阻容执行[RC-20261117.1](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)：全部使用本机正式库，不改图形/针距冒充；横件Ref/Value都横、竖件两者都竖；文字沿各自引脚线齐平、落网格、无干涉。确定器件方向后不得自由选择字向。先验证一横一竖的原生小样，保留物理脚号与极性。
4. 读取库针距、工程单位及实际热点。本用户参考1/2 of pin to pin，换工程重新换算；符号针距须与网格相容。修针距/原点时协调全部相关线和端口，不能逐对象盲目取整。
5. MOS使用真实G/S/D、沟道、体二极管及并脚/双管结构；BJT保留内部图形和箭头，不用空矩形代替离散器件。优先复用本机正确符号，逐脚核对封装。
6. [端部对象](../../notes/capture_endpoint_symbols_standard.md)先校准批准原生正例的库图形、实际热点、变换和Name显示框架，再按实际轮廓验方向：左端Name >>——电路，右端电路——<< Name；自身Name完整可见且在外侧，不能用线内Alias替代。电源保留合理母线短竖支路；GND完整可辨，实际bar长度由电路向外递减，左右镜像；不能以glyph外置代替此内部方向检查。电源自身Name须紧贴符号外侧，核字形实际间距上限；GND是否显示Name依批准profile，若显示也须自身Name外置。保持原生类型、作用域及两端连接，完整图形/字形和邻接场景均要验，不能靠固定rotation/mirror猜。
7. 普通页内标识按[真实WireScalar/Alias规则](../../notes/capture_wire_net_alias_standard.md)；Alias的锚点在线规则不适用于端部Name。不能用注释、空白端口或悬空Name冒充连接，不能为增加标签凭空增加连接点。文字移动与电气热点移动分开核对。
8. 先保留清楚电路，按功能紧凑成组，后调整显示文字、端部符号，再按完整字形/本体/网格选择适当短的局部引线；不为填页面或表格列宽保留过长直线。逐组核行列间距、逐支路核实际线长和更短可行候选/必要约束；不制造长U形回折、跨区穿表或文字挤压。依据[真实文字/原生变换](../../notes/capture_text_geometry_and_readability.md)建立完整模型，未知锚点列例外。
9. 在副本应用后原生保存、独立重开，逐Ref.Pin比较网络、NC、Value/装配/封装/关键属性；逐页检查完整元件、引脚、文字、电源/翻页、Alias、导线和表格。有PCB交接时核主工程新鲜网表和严格导入。显示、离线模型、GUI、PCB分别报告；网表等价不能替代可读性验收。
10. 用户要求后台时不启动或激活GUI。任一已知可见问题或关键显示对象漏验时，不把候选回写为“全页排版完成”。学习规则同步不能冒充实际工程整改完成。

[阻容细则](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)及[整体方法](../../notes/capture_compact_schematic_layout_method.md)的方向与对齐要求统一以RC-20261117.1为准。学习技能须后续读取复用，不等于模型权重训练或自动加载。

后台核准可按[原生打印、真实字形与纸张边界](../../notes/capture_native_print_text_and_canvas.md)执行；只复用当前宿主/自有副本并验证非空输出及来源。页面原生单位/可绘范围逐页实读，不能用截图或固定比例猜；零框、未知字体、内容超边/裁切和缺项不通过。整页平移不能再次改变本地引脚/文字坐标，扩大纸张不代替功能紧凑度。
