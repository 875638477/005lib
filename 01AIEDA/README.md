# Cadence 学习库

本目录保存学习笔记、课程索引、可复用方法、源脚本及最终示例。

- [使用规则与方法](EDA.md)
- [课程索引](课程索引.md)
- [学习记录入口](study_records/README.md)
- [公共库维护索引](library_maintenance/INDEX.md)
- [2026-11-08 整理报告](library_maintenance/20261108_cleanup/REPORT.md)

正式器件库统一从相邻目录使用：`../01Pad_lib`（PAD）、`../01Psm_lib`（DRA/PSM/形状）、`../01captureLib`（OLB）、`../01_3d_step_lib`（STEP）。已核对一致的库副本已移除；不同内容和教学专用的示例资产保留。被移出的旧版本、制作日志与试验目录有本地压缩备份及逐文件 SHA256 清单。

旧脚本和机器清单中的历史路径、验证结论按当时记录理解。旧试制目录、暂存依赖或日志已归档，历史制作脚本不保证能直接重跑；最终示例的原生设计文件仍保留。重跑工具如需 numpy 或 PyMuPDF，请在项目虚拟环境中安装所需依赖。

说明文档使用 `.md`。`.txt` 仍可能是 DEVICE 器件定义、Extract/BOM 模板、数据或运行输出，不能按后缀统一删除。`.opj` 是 Capture 项目文件。清理脚本保护整个 `01AIEDA`，不再递归扫掉学习资料。

本轮只做文件核验、整理及 GitHub 同步，没有启动 Cadence 重新验证工程或 3D 装配。
