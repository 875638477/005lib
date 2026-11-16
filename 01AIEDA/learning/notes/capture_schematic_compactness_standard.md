# 原理图功能组紧凑度与适当短引线

现行版本：**COMPACT-20261116.1**。这是独立的强制验收层，与[RC-20261116.2](capture_rc_horizontal_ref_value_layout_spec.md)、[端部对象规范](capture_endpoint_symbols_standard.md)及原理拓扑要求取 AND；原 RC 五项检查器不检查紧凑度。碰撞为零、方向正确或网表相同，都不能证明本规范通过。[机器 contract](capture_schematic_compactness_contract.json)及[只读 facts 检查器](../scripts/capture_layout_tools/check_schematic_compactness.py)配套使用。

## 三项优先规则

1. **先定本功能组，再紧凑成行。** 识别串联耦合、差分对、同组去耦、分压/反馈等局部功能；同功能组的本体、Ref、完整 Value、针名针号、网名及端符一起形成真实字形包络。同组行距由这些包络和必要邻域间隔决定，不按页面高度均分，不把功能不同的组压成一个机械表格。不同组保留有理由的阅读留白。竖置组在其局部坐标系执行同一原则，不能为了成行拆散反馈或去耦关系。
2. **器件至端符采用适当短引线。** 按实际针热点到真实端符/支路热点的连续路径测量；共线段先归一，不按数据库 wire 数量判断长短。端符、完整 Name、Ref/Value 都应在本局部功能的近邻。允许同组按最长必要文字、真实热点、引脚分组及共同短列选择统一引线长度，并留少量网格余量；不得为全页列宽或铺满画布把端符拉远。长直线没有折弯也可能不合格。
3. **先满足原理与完整可读，再压缩空白。** 必须保持正式库、Ref/Value 字向与引脚线关系、物理脚号/极性、电气及作用域。先选用户批准的原生端符姿态和局部连接表达，再选间距；GND/电源与文字冲突不能只靠不断向远处拉线解决。比较合理姿态、局部短支路及确实同网时的简洁母线；图形、全部字形和邻行导线都需检查。任何更短方案若引入碰撞、割裂功能或损害归属，即不可采用。

## 可执行的预算与验收

- 从当前保存版本读取 PinToPin、单位、捕捉分数和网格原点，得到 `g`；不能把某工程的 5 文档单位或固定 mil 当通用阈值。所有行距和路径长度同时记录文档单位及 `/g`。
- 按完整实际字体字形、真实符号/热点及邻域建立同源 scene。不能以字符数、脏 bbox 或缩字代替。完整字形缺失、局部功能未分类、预算无来源均为 UNKNOWN。
- 每个组给出一个有证据的**合理紧凑候选预算**：来自适用的批准原生参考，或独立验证的局部候选。候选须包含真实行锚点和引线路径，完整邻域无干涉，原理、电气与作用域保持，写明哪些文字/热点/对齐约束决定长度与行距。检查器不自动排版、不搜索全局最短，也不替代工程整改。
- 同组共同短列的预算按该候选中最长必要引线核定；不能借组内共同列把其它模块的最长字符串扩成全页预算。预算在合理候选之上默认只允许 **1 个当前网格**的阅读/取整余量；确有局部功能理由可到 **2 个网格**，须绑定理由与约束证据。这是候选预算的余量，不是所有工程的固定间距，也不要求不惜代价机械最短。
- 对实际每对组内相邻行，核其局部法向间距不超过该候选对应间距加批准余量；对每条器件至端符路径，核其长度不超过该候选的相应或组内共同短列预算加批准余量。超出判 FAIL；缺预算/候选/完整邻域或来源判 UNKNOWN，不能仅写“紧凑”。用户锁定的布局或保持反馈结构等限制必须进入候选预算，不可无理由豁免。
- 串联 USB3 正/负、PCIe 正/负耦合电容分别按各自信号功能成对组织，短直连接、同组紧凑；不同接口组可有适量分组留白。示例中的 Ref、绝对坐标、物料及电气定义不是其它工程输入。

2026-11-16反例暴露的是：少量串联电容虽然横字、两端 Name 正确，也能被固定的大行距和两侧长直线铺满大片画布。既有“短直线/紧凑”文字没有长度事实门禁，故新增本层。用户截图仅作为该工程本地审核证据，不随通用规则公开。

## 事实与交付边界

使用同一保存重开的 XML、原生数据清单及其哈希，分别绑定功能组盘点、实际几何/网格、完整字形 scene、拓扑和合理候选预算。事实须覆盖本次范围的全部 R/C 功能组及相关器件至端符引线；漏项为 UNKNOWN。`check_schematic_compactness.py` 只核这些独立证据及预算，不把自填 PASS 当证据，不产生任何 CAD 修改。

最小输入约定如下，完整的抽象样本由规则门禁负测生成在工程审核目录，不包含真实设计截图或工程数据：

- facts 顶层：`rule_version`、`source_xml={path,sha256}`、`native_manifest={path,sha256}`、独立盘点的 `expected_group_ids` 与本次 `group_ids`、`evidence`。证据路径可绝对或相对 facts 目录。
- 原生清单：绑定 `source_xml_sha256`，`files` 中逐项给 `role/path/sha256`，至少覆盖 pins、wires、net_symbols、part_frames、page_grid。其余五份证据同时绑定 XML SHA 和 `native_manifest_sha256`。
- `inventory`：`expected_rc_ids` 及 `groups`，每组含 `id/functional_role/member_ids/row_ids/lead_ids`。`geometry`：当前 `grid`（units、grid_doc、pin_to_pin_doc、selected_fraction、origin）及按组的原生局部 `frame={origin,u,v}`、有序 `rows={id,anchor}`、实际 `leads`。
- 每条 lead：稳定 `id/owner_pin_id/terminal_id/terminal_kind/net_identity` 与按连通顺序给出的 `path` 点列；当前与候选身份必须一致，路径在网格且连续正交。导出器负责以原生连接关系归一共线段，不能直接把任意同名线拼成路径。
- `scene` 每组：`full_ink/neighborhood_complete`、覆盖的 member/lead 身份、全部 `covered_roles`、`unresolved_objects/interferences`、测得及要求的 clearance。`topology` 每组绑定功能审核理由、成员、保留结果和 `constraint_ids`。
- `budgets` 每组：`basis_id/source_kind/reason/reason_codes/constraint_ids`、整数 `slack_grids`、可选局部 `shared_lead_classes`，以及该合理联合候选的有序 rows/实际 leads、独立功能/电气作用域验证和完整 scene。共享类仅限本功能组；候选的完整路径长度及行间距就是预算基准，不能另填一个无候选支持的巨大上限。

最终要求为 RC 五项 AND COMPACT AND 端部/拓扑/网格及电气相应门禁。声明只限已核范围；保存规则不等于已整改当前原理图，原理图线短也不代表 PCB 已布线或满足物理长度约束。PCB 是否可移动或走线由当前用户任务单独确定。
