# Capture 导线网络别名标准（Net Alias）

更新：2026-11-11。用户明确指定为后续全部非翻页网络标识的默认规则。项目的 DSN、快照、脚本、日志、路径和验收证据放项目根；本学习库只保存通用规则。

| 项目 | 统一标准 |
| --- | --- |
| 学名与对象 | 导线网络别名，OrCAD Capture 的 **Net Alias**。原生 XML 中为 `WireScalar/Alias`，属于实际承载导线。 |
| 参考关系 | 用户指定既有原理图 P33.BTB 上的导线别名作为正确参考：锚点落在导线上，完整文字水平、紧挨导线上方。坐标随本地导线确定，不复制实例的绝对位置。 |
| 水平导线 | `Alias.locY` 等于承载导线 Y，`locX` 落在线段范围内，`rotation=0`。文字使用 Alias 原生基线关系，不能把锚点上移成悬空文字。 |
| 字体参考 | 已从用户指定的原生对象读取：Arial，LOGFONT `height=-9`、`width=4`、`weight=400`、`italic=0`、`orientation=0`、`escapement=0`。这些是数据库字体参数，不直接称作 GUI 的“9 pt”。换版本、字体或符号后重新核对显示。 |
| 与阻容文字关系 | 左网名 → 位号 → 器件 → 完整 Value → 右网名；位号/Value 仍紧挨两侧引线上方。网名避开位号、Value、引脚号、器件和邻线，不能把位号/Value挪远来迁就网名。 |
| 非水平导线 | 优先使用同一实际导线连接片段上的现有水平段；确需补短水平引线时在本地处理并验证网络。不能把别名移到附近但不相连的线段。 |
| 无承载导线 | 先检查是否直接接器件引脚。确有电气用途时补受控短引线；无导线、无引脚、无实际作用的孤立重复标识可在记录并验证后删除。 |
| 真正的跨页/层次/电源对象 | 保留其原生类型、网络名称、真实热点和作用域。普通页内标识用 Alias；跨页连接和电源全局连接仍由真实、可见的连接符表达。 |
| 禁止的替代 | 禁止用普通注释、移动的 Name 显示属性、空白/无图形 OffPageConnector、空白 Global 或自制透明端口冒充导线别名。即使暂时网表相同也不符合本标准。 |
| 连接点 | 单纯放置网名不应凭空增加连接点圆点。真正 T 分支、引脚和连接交点按实际拓扑保留；不得只为去掉圆点删除真实连接。 |
| 验收 | 原生保存重开，确认 Alias 仍属于正确导线且锚点在线上、完整网名未改、图形可读；对全部物理 RefDes.Pin 比较网络分组、每脚网名和 NC，同时检查器件属性/封装。仅比较网络数量不够。 |

[通用矢量示例](assets/capture_wire_net_alias_standard.svg)表达对象及位置关系，不是工程截图。

```xml
<WireScalar>
  <Defn dbId="..." startX="100" startY="200" endX="300" endY="200" name="" />
  <!-- 其他原生导线字段略；完整格式以本机官方导出为准 -->
  <Alias>
    <Defn color="48" dbId="..." locX="120" locY="200" name="NET_SIGNAL" rotation="0" />
    <AliasFont>
      <Defn escapement="0" height="-9" italic="0" name="Arial" orientation="0" weight="400" width="4" />
    </AliasFont>
  </Alias>
</WireScalar>
```

例中 20 个文档单位的水平偏移只用于说明锚点关系，不是固定模板。缓存 `AliasBBox` 可能为零，不能用零包围框判断没有文字或没有重叠。

批量实施先记录原对象种类、页名、网络、真实热点、所属导线和字体。将错误的空白端口改成 Alias 后，可能暴露其原本承担的跨页或电源全局连接；必须按真实连接范围补回合适的**可见原生连接符**，再导出完整 PST 比较。不得为了通过网表比较又把 Alias 换回空白端口。

原生/几何检查、离线预览与 Capture GUI 阅读检查分别记录；后台处理不抢窗口，也不把离线预览写成 GUI 已验。

配合[网络名称细则](capture_netlabel_layout_rules.md)、[整体方法](capture_compact_schematic_layout_method.md)及[阻容文字规范](capture_rc_horizontal_ref_value_layout_spec.md)执行。此文件需由后续会话读取复用，保存不等于模型训练或所有会话自动加载。
