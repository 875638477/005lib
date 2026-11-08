# Allegro 17.2：封装改形后的 3D 核对、保存与跨电脑复用

库根当前为 `D:/005lib`。换电脑或盘符后，从实际库目录运行 `01env/setup_library_paths.cmd`；LGA1700 三板使用 `setup_library_paths.cmd -Lga1700`。安装脚本按自身位置解析库根，备份已有 env，再写入当前电脑的配置，不修改系统 HOME。

## 先核对定义，再添加模型

修改二维封装不会自动修改 STEP。名称相同也不能证明尺寸、原点、针序、安装孔相同。比较当前 PCB 缓存和库 DRA 的完整针号、焊盘名、位置、转角及机械属性；不同定义保留独立路径，不能拿通用库覆盖项目变体。

PSM、PAD、STEP、脚本分别来自 `01Psm_lib`、`01Pad_lib`、`01_3d_step_lib`、`01script_file`。三板专用查找顺序为 MOD26 变体、MOD31 变体、通用 PSM。这个顺序只用于相应项目，不能把自定义金手指针序当作通用内存连接器使用。

用 STEP 实际实体检查长宽高、单位、板面方向、底面高度和插针位置。翻面、旋转及封装原点偏移都必须计算。尤其不能把模型包围盒中心一律放到封装原点：有些封装原点在第一个焊盘附近。安装孔和 PCB 铜皮金手指没有额外器件外壳，不应随意附加螺钉或连接器模型。

原厂 CAD、已有参考 CAD、按封装轮廓建立的参考包络应分别记录。`REF_ENVELOPE_*` 是可显示的参考包络；未获得厂家尺寸图时，估计高度必须在清单中标记为 `reference_estimate`，不能用来宣称干涉、装配高度或制造验收通过。

## 必须走完的持久化链路

1. 在自有 DRA 副本的 Package Symbol 编辑器中添加真实 STEP。先用完整输入路径；已在 17.2 S045 核对保存的 `step_name` 只保留文件名，换电脑依靠 steppath 查找。
2. 调整旋转、XY 偏移和 Z 高度。`axlStepSet` 的偏移单位是当前 DRA 单位；mm 与 mil 必须换算，不能把 mm 数值直接写进 mil 封装。
3. 保存 DRA，关闭/重开后用 `axlStepGet` 核模型文件名、旋转和偏移。只在当前会话里能看到模型不足以证明保存成功。
4. 执行 File → Create Symbol，或 `axlCompileSymbol`，重新生成 PSM。仅保存 DRA 而没有重编 PSM，会使 PCB 继续读取旧模型。
5. 在 PCB 副本执行 `refresh_symbol -g -p -m 输入.brd 输出.brd`。`-g` 表示只更新 STEP 映射。`01script_file/update_pcb_3d_only.ps1` 实现这个流程，默认生成新文件并保留输入。
6. 保存并原生重开 PCB，核每个实际 symbol definition 的模型与姿态；再核引脚网络、焊盘、位置、正反面、走线、板框、层叠和有效约束未改变。

已有 `place_updata_3d.scr` 是 GUI 的 Update Symbols + Reset STEP Mapping 录制流程，会涉及封装刷新；单纯更新模型优先使用上述 `-g` 工具。

可复用函数位于 `01script_file/bind_current_package_step.il`，只对已打开的封装副本操作。调用前确认输出 DRA 是你要保存的文件，因为保存和编译会覆盖该输出。

## GitHub 同步

将 DRA、PSM、相对文件名引用的 STEP、PAD 依赖、安装脚本和教程一起同步。只上传 BRD 或 DRA 不能让另一台电脑完整复用。当前远端为 `875638477/001Diy` 仓库中的 `005lib` 子目录，本机库从旧路径迁出并不改变这个远端层级。

冻结 SHA 清单后构建提交；正常推送，不强制覆盖远端。独立读回远端文件树并核 Git blob SHA，目标子目录之外的文件保持不变。任务专用工程、PCB、日志和最终服务器回执保存在工程根，公共库的定义维护与必要备份保存在 `01AIEDA/library_maintenance`。

本机接口依据：Cadence 17.2 `share/pcb/examples/skill/DOC/QIR/NEW/axlStepGet.txt`、`axlStepSet.txt`、`FUNCS/axlSaveDesign.txt`、`axlCompileSymbol.txt`，以及 `share/pcb/batchhelp/refresh_symbol.txt`。此教程说明通用操作，不代表任何具体 PCB 已完成机械或电气验收。
