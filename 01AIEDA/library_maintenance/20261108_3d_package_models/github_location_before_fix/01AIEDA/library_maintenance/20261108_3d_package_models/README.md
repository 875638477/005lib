# 封装 3D 与原名库更新（2026-11-08）

库根为 `D:/005lib`。115 种在用封装按既有路径和文件名更新 DRA/PSM，没有添加替代封装名；原有 MOD26 四个版本继续保留。曾因无针号机械孔枚举次序误报的 13 项已撤回；完整多重集合复核没有发现这些针定义差异，不新增 MOD31 目录。

100 种有实体器件的封装绑定 STEP；15 种测试点、安装几何、铜皮金手指等不附加外壳。新增 74 个模型中，73 个 REF_ENVELOPE 是参考包络，估计高度在 PACKAGE_3D_MANIFEST.csv 明确标注；1 个是保留解析曲面的 CPU 插座与扣具组合坐标转换版。参考包络不作为厂家机械图、干涉或制造放行依据。

最终流程：在有效 PCB symbol definition 绑定真实 STEP，保存/重开逐项验证；原生 dump_libraries -a -c -x 导出 DRA/PSM；再用这些 PSM 对副本 refresh_symbol -g，并重开检查模型名、旋转、偏移及全部二维/约束保持。S045 setter 的内部 mil 输入与 getter 的当前显示单位必须分开处理。

跨电脑运行 01env/setup_library_paths.cmd；相应三板使用 -Lga1700，沿用 MOD26、通用库的既有顺序。教程：01AIEDA/learning/notes/allegro_step_model_persistence_and_portable_paths.md。GitHub 同一路径同名文件更新内容，新增 STEP 和依赖一并同步。

before/ 保留原库；intermediate_v1/ 是本轮未通过最终 PCB 偏移验收的中间生成件及已撤回目录，不能作为当前库入口。FINAL_INSTALL_MANIFEST.json 与 FINAL_MODEL_HASHES.json 标识当前文件。实际 PCB、逐针及服务器最终回执保存在工程根。
