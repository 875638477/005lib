# 本机EDA公共库的现行规则入口

本目录维护本机正式公共库。Cadence原理图新建、整理及库复用任务先读01AIEDA/learning/skills/cadence-schematic-layout/SKILL.md，并读取其链接的RC-20261116.2规范和机器规则。原理图排版不能仅以网表/碰撞检查代替本机库、文字方向及引脚线对齐核验。

先核远端与本机身份，保护本机不同版本，不盲目覆盖；学习库只存可复用方法/工具，工程源件、专用脚本、日志、校准与审核留工程。其他类型任务按自身规则执行。


## 端部符号与既有接插件优先规则（2026-11-16）

本节仅适用于本仓库的EDA原理图设计、重建和排版任务；其他类型任务仍按原有规则。任务开始先读[ENDPOINT-20261116.1](01AIEDA/learning/notes/capture_endpoint_symbols_standard.md)、[机器contract](01AIEDA/learning/notes/capture_endpoint_symbols_contract.json)及其[用户正反例](01AIEDA/learning/notes/assets/endpoint_symbols_20261116/README.md)，再按上文技能入口执行现行R/C与整体方法。

翻页符/电源必须保留真实对象自身有效可见Name并放外侧，不能隐藏后用线内Alias或文本替代；翻页符按实际变换后的轮廓朝向所属电路内侧，不能凭颜色或固定rotation/mirror判断。电源保留合理母线短竖支路；GND使用完整真实接地图形，是否显名依批准原生profile，若显名也用自身Name外置。先校准原生正例，再批量应用；缺事实为unknown，端部facts checker或R/C/PST通过不能替代全量验收。

冻结并保留全部既有接插件位号/实例，包括额外、未用、预留及仅某方案使用者；未经用户对具体对象明确许可不得删除。已授权调位/增针/逐针修改不扩大为删除权限，交付前逐位号比较并确认真实实例仍在。

此入口为仓库内可读取规则；保存或同步不代表所有旧对话即时加载，也不修改本机全局Codex规则。
