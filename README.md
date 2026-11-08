# 005lib

EDA 工程库。正式本机根目录：`D:\005lib`。

<!-- 004Document:begin -->
## 004Document 器件资料库

[打开 004Document](004Document/) · [整理报告](004Document/004Document_整理报告_2026-11-07.md) · [待核实清单](004Document/004Document_待核实_2026-11-07.md)

本机位置：`D:\005lib\004Document`。共 6,145 个文件，含 5,115 份 PDF 及配套资料，保留 28 个原有大类，按“大类 / 编号子类 / 供应商”整理。例如：`003Inductor/00贴片功率电感/3L COILS(台湾三礼)`。

SHA-256 完全相同的 898 份 PDF 副本已去重；内容不同的版本保留。供应商未知 439 份、仍需结合原目录复核 750 份、PDF 解析异常 2 份，详情见待核实清单。

文档和二进制资料中不小于 64 KiB 的文件使用 Git LFS；完整下载需要安装 Git LFS，并在克隆后执行：

```sh
git lfs install
git lfs pull
```

GitHub 目录树及全部 5,646 个独立 LFS 对象均已核对。
<!-- 004Document:end -->

| 目录 | 用途 |
| --- | --- |
| 01AIEDA | Cadence 自动化、库维护和学习资料 |
| 01Pad_lib | 焊盘库 |
| 01Psm_lib | PCB 封装库 |
| 01_3d_step_lib | STEP 模型 |
| 01aeLib | CIS 数据库配置与库 |
| 01brdlib | PCB 模板和板库 |
| 01captureLib | 原理图库 |
| 01env | Cadence 环境配置和快捷方式模板 |
| 01script_file | Allegro 脚本 |
| 004Document |  规格书 局部驱动 |



## 2026-11-08 学习库整理

[学习库入口](01AIEDA/README.md) · [清理与同步报告](01AIEDA/library_maintenance/20261108_cleanup/REPORT.md) · [安全清理脚本](01script_file/Clean-EDA.ps1)

学习笔记、方法、源脚本及最终示例保留；历次制库暂存、备份和已核实的重复库副本已移出工作树。正式焊盘、封装和 STEP 库仍分别位于 `01Pad_lib`、`01Psm_lib`、`01_3d_step_lib`。

说明类 TXT 已改为 Markdown。器件定义 TXT、BOM/Extract 模板、引脚数据、CSV/XML 及 Capture 项目 OPJ 按用途保留。`0000Ad_Keil_Cadence...bat` 默认预览，显式 `/apply` 才清理；整个学习库和正式库受保护。本地恢复备份不上传 GitHub。
