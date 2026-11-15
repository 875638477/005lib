---
name: cadence-schematic-layout
description: Preserve readable circuit topology while arranging Cadence Capture parts, grids, reference designators, values, net aliases, power symbols and off-page connectors. Use for reference-module integration or schematic cleanup with local real symbols and native save/reopen plus connectivity verification.
---

# Capture 原理图可读性、功能拓扑与网格

实施前读[用户2026-11-15现行规则与正反例](../../notes/capture_topology_preservation_and_grid.md)。当前用户约束优先；旧笔记中“一律横放”不再适用。

1. 固定最新源件、参考图、库、PinToPin/捕捉网格、可移动范围和SHA。实际工程的输入、脚本、日志、审核和交付在工程根，学习侧只放通用方法及获准示例。
2. 用户提供已优化原图时，直接复用其功能模块组织、横竖方向、引脚分组、直接导线、反馈/补偿/去耦及启停关系，按新需求局部融合。不可只抄BOM后把阻容拆成孤立网名行；不恢复已删除器件或参考中的已知电气错误。
3. 横放、竖放阻容均保持位号左、完整Value右，文字水平、归属明确、锚点落工程网格；按真实字形避让本体、针号、网名、导线及邻近文字。保留脚号、极性和Kelvin关系，不为脚序或几何整齐强转器件。按功能分组对齐，不以齐表破坏电路。
4. 读取库针距、工程单位及实际热点。本用户参考1/2 of pin to pin，换工程重新换算；符号针距须与网格相容。修针距/原点时协调全部相关线和端口，不能逐对象盲目取整。
5. MOS使用真实G/S/D、沟道、体二极管及并脚/双管结构；BJT保留内部图形和箭头，不用空矩形代替离散器件。优先复用本机正确符号，逐脚核对封装。
6. [电源、GND与翻页符](../../notes/capture_topology_preservation_and_grid.md#电源与翻页符按导线末段自然平接)沿所属导线末段平接，名称在外侧；水平左端翻页尖头向右进电路，右端相反。不为默认朝向加多余竖折线。按真实热点和Name偏移旋转，保持Global/OffPage原生类型、作用域及两端连接。
7. 普通页内标识按[真实WireScalar/Alias规则](../../notes/capture_wire_net_alias_standard.md)；不能用注释、空白端口或悬空Name冒充连接。文字移动与电气热点移动分开核对。
8. 先保留清楚电路，后调整显示文字、端部符号，最后做确有需要的局部短线；不制造长U形回折、跨区穿表或文字挤压。依据[真实文字/原生变换](../../notes/capture_text_geometry_and_readability.md)建立完整模型，未知锚点列例外。
9. 在副本应用后原生保存、独立重开，逐Ref.Pin比较网络、NC、Value/装配/封装/关键属性；逐页检查完整元件、引脚、文字、电源/翻页、Alias、导线和表格。有PCB交接时核主工程新鲜网表和严格导入。显示、离线模型、GUI、PCB分别报告；网表等价不能替代可读性验收。
10. 用户要求后台时不启动或激活GUI。任一已知可见问题或关键显示对象漏验时，不把候选回写为“全页排版完成”。学习规则同步不能冒充实际工程整改完成。

[阻容细则](../../notes/capture_rc_horizontal_ref_value_layout_spec.md)及[整体方法](../../notes/capture_compact_schematic_layout_method.md)的旧方向表述以2026-11-15现行规则为准。学习技能须后续读取复用，不等于模型权重训练或自动加载。
