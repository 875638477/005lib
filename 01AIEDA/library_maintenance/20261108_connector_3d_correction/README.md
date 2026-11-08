# 连接器三维纠正（2026-11-08）

当前库根 D:/005lib；GitHub https://github.com/875638477/005lib。沿用已有 DRA/PSM/STEP 原路径原文件名更新，原 MOD26 特殊版本优先级保留。

上一轮仅有模型绑定，若干连接器只是整块包络，甚至因未取到 PLACE_BOUND 而退化到 1 mm。此次新增槽口、接触针、金属壳、定位缺口和端部保持结构，按本轮实际焊盘坐标布置。DDR4 大座子原模型的焊接脚位于负 Z，原 X=180 度导致倒置；现 X/Y/Z=0，偏移 (0.0178,-0.0013,21.06) mm，塑胶底落于板面、尾脚向下。288 个焊接端与实际焊盘最近点坐标复核在附带 JSON，应用偏移后的最大中心误差约 0.011 mm。PCIe 原 PLACE_BOUND 相对针脚错位约 31 mm，模型改以实际针脚阵列确定位置；未改动二维封装。

**DDR4 大座子沿用已有详细模型；另 9 个模型是明确标记的详细显示参考，不是厂家原始 CAD。模型端子按原封装放置，部分本体高度、壳体、锁扣、键位细节仍为参考。用于显示，不能单凭此资料进行装配或采购验收。** 厂家 TE 公开 CAD 下载本轮返回 HTTP 403，没有把未获取的模型冒称为厂家模型。

| 封装 | 位号 | STEP |
|---|---|---|
| Z690I_ATX_12V_2X4 | ATX_12V_2X4 | REF_ENVELOPE_Z690I_ATX_12V_2X4.step |
| Z690I_DDR4_SOCKET | DDR4_A1,DDR4_B1 | ddr4_dimm_288p_pc.step |
| PIN10_2D54MM_2X05 | F_PANEL | REF_ENVELOPE_PIN10_2D54MM_2X05.step |
| Z690I_PCIEX16 | PCIEX16 | REF_ENVELOPE_Z690I_PCIEX16.step |
| LP380_G120_B0R01_SOCKET | J_SIO | REF_ENVELOPE_LP380_G120_B0R01_SOCKET.step |
| SMD-NGFF-75P-M2-H3-LOTES | MD2 | REF_ENVELOPE_SMD_NGFF_75P_M2_H3_LOTES.step |
| SODIMM_DDR4_260P_H | J_PCH | REF_ENVELOPE_SODIMM_DDR4_260P_H.step |
| HEADER10X2_2MM_USB3_A | F_U32 | REF_ENVELOPE_HEADER10X2_2MM_USB3_A.step |
| Z690I_USB20 | USB1 | REF_ENVELOPE_Z690I_USB20.step |
| SMD-NGFF-75P-M2-H3-LOTES_2242_2230 | PCH_M2 | REF_ENVELOPE_SMD_NGFF_75P_M2_H3_LOTES_2242_2230.step |

流程：在有效 PCB symbol definition 上绑定，保存并原生重开；dump_libraries 导出 DRA/PSM；在修改前 PCB 的副本上从新 PSM 仅刷新 STEP；原生重开核对针、网、二维几何、布局、走线、板框及全部有效约束；之后按哈希保护安装，再从实际库路径刷新正式 PCB。模型存在和非空包围盒不能代替朝向、接触对位及显示检查。

其他电脑更新 GitHub 后执行 01env/setup_library_paths.cmd -Lga1700；已有旧 PCB 执行 01script_file/update_pcb_3d_only.ps1 -Board <PCB> -Lga1700。关闭并重新打开旧 3D Canvas，使用新生成 PCB；不要继续看已经打开的旧缓存。

build_connector_models.py 是工程内复现源件，依赖本工程 native.py 和保留的上一轮原生扫描；不是对任意工程运行的通用生成命令。before/ 保留原名旧内容，INSTALL_MANIFEST.json 记录本轮安装文件。项目侧完整证据见 32_z690i_connector_3d_fix_20261108。
