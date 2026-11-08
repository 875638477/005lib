# Allegro 17.2：封装改形后的 3D 核对、保存与跨电脑复用

库根当前为 `D:/005lib`。换电脑或盘符后，从实际库目录运行 `01env/setup_library_paths.cmd`；LGA1700 三板使用 `setup_library_paths.cmd -Lga1700`。安装脚本按自身位置解析库根，备份已有 env，再写入当前电脑的配置，不修改系统 HOME。

## 先核对定义，再添加模型

修改二维封装不会自动修改 STEP。名称相同也不能证明尺寸、原点、针序、安装孔相同。比较当前 PCB 缓存和库 DRA 的完整针号、焊盘名、位置、转角及机械属性；不同定义保留独立路径，不能拿通用库覆盖项目变体。

PSM、PAD、STEP、脚本分别来自 `01Psm_lib`、`01Pad_lib`、`01_3d_step_lib`、`01script_file`。三板沿用 MOD26 变体、通用 PSM 的既有顺序。本轮 DRA/PSM 保持原目录原文件名；同路径同名文件在 GitHub 更新内容，不改名创建替代封装。

无针号机械孔必须按实际坐标、焊盘名及机械属性匹配，不能用数据库枚举序号当作身份。一次检查曾因此把 13 种封装误判为针定义不同，完整多重集合匹配后已撤回；这不是 13 种针序错误，也不据此增加目录或改名。

用 STEP 实际实体检查长宽高、单位、板面方向、底面高度和插针位置。翻面、旋转及封装原点偏移都必须计算。尤其不能把模型包围盒中心一律放到封装原点：有些封装原点在第一个焊盘附近。安装孔和 PCB 铜皮金手指没有额外器件外壳，不应随意附加螺钉或连接器模型。

原厂 CAD、已有参考 CAD、按封装轮廓建立的参考包络应分别记录。`REF_ENVELOPE_*` 是可显示的参考包络；未获得厂家尺寸图时，估计高度必须在清单中标记为 `reference_estimate`，不能用来宣称干涉、装配高度或制造验收通过。

## 必须走完的持久化链路

1. 在自有 DRA 副本的 Package Symbol 编辑器中添加真实 STEP。先用完整输入路径；已在 17.2 S045 核对保存的 `step_name` 只保留文件名，换电脑依靠 steppath 查找。
2. 调整旋转、XY 偏移和 Z 高度。mm 与 mil 必须换算。本机 S045 的实测链路中，直接设置部分 DRA 的偏移后经 PSM 刷新到 PCB 出现 0.0254 倍缩放，不能只依据接口文档或 DRA 会话内读值报通过。可靠的替代路径是在 PCB 自有副本的有效 symbol definition 上绑定，保存并重开核对，再用 `dump_libraries -a -c -x` 原生导出 DRA 和 PSM，最后仅模型刷新回新 PCB 验证偏移和几何。不能通过改动原封装工作单位来掩盖偏移错误。
3. 保存 DRA，关闭/重开后用 `axlStepGet` 核模型文件名、旋转和偏移。只在当前会话里能看到模型不足以证明保存成功。
4. 执行 File → Create Symbol，或 `axlCompileSymbol`，重新生成 PSM。仅保存 DRA 而没有重编 PSM，会使 PCB 继续读取旧模型。
5. 在 PCB 副本执行 `refresh_symbol -g -p -m 输入.brd 输出.brd`。`-g` 表示只更新 STEP 映射。`01script_file/update_pcb_3d_only.ps1` 实现这个流程，默认生成新文件并保留输入。
6. 保存并原生重开 PCB，核每个实际 symbol definition 的模型与姿态；再核引脚网络、焊盘、位置、正反面、走线、板框、层叠和有效约束未改变。

已有 `place_updata_3d.scr` 是 GUI 的 Update Symbols + Reset STEP Mapping 录制流程，会涉及封装刷新；单纯更新模型优先使用上述 `-g` 工具。

可复用函数位于 `01script_file/bind_current_package_step.il`，提供 `bindBoardPackageStep(refdes stepFile poseMM)`，只对已打开 PCB 的真实封装定义操作，poseMM 为 `(rx ry rz ox oy oz)`。调用后还必须保存自己的 PCB 副本、原生重开，导出库，并验证 PSM 重新载入；仅函数返回成功不足以验收。

## GitHub 同步

将 DRA、PSM、相对文件名引用的 STEP、PAD 依赖、安装脚本和教程一起同步。只上传 BRD 或 DRA 不能让另一台电脑完整复用。实际现用远端为 [875638477/005lib](https://github.com/875638477/005lib) 的仓库根，九个01目录直接位于根。2026-11-08从服务器重新核实，旧 001Diy/005lib 子目录登记已过期；不要套用旧工具的 --prefix 005lib 造成再次嵌套。

冻结 SHA 清单后构建提交；正常推送，不强制覆盖远端。独立读回远端文件树并核 Git blob SHA，目标子目录之外的文件保持不变。任务专用工程、PCB、日志和最终服务器回执保存在工程根，公共库的定义维护与必要备份保存在 `01AIEDA/library_maintenance`。

本机接口依据：Cadence 17.2 `share/pcb/examples/skill/DOC/QIR/NEW/axlStepGet.txt`、`axlStepSet.txt`、`FUNCS/axlSaveDesign.txt`、`axlCompileSymbol.txt`，以及 `share/pcb/batchhelp/refresh_symbol.txt`。此教程说明通用操作，不代表任何具体 PCB 已完成机械或电气验收。


## 2026-11-08 连接器显示纠正

仅 axlStepGet 读到名称不代表模型正确。必须检查实际本体、插槽开口、针脚方向、贴板高度，以及模型接触端与编号焊盘的对位；机械孔/PLACE_BOUND 本身也可能错位，不能无条件按最大外框生成包络。没有外框时不得默认为 1x1 mm 还报告“模型完整”。DDR4 ddr4_dimm_288p_pc.step 原来负 Z 的脚朝下，需保留零旋转并按本体底面 +21.06 mm 校正高度，不可 X=180 度翻转。当前 10 个连接器更新及显示参考限制见 ../library_maintenance/20261108_connector_3d_correction/README.md（相对本学习笔记应使用 ../../library_maintenance/20261108_connector_3d_correction/README.md）。
