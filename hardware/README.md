# 硬件目录说明

```text
hardware/
  schematic/
    dual-cage-block.svg          # 系统框图
    dual-cage-schematic.svg      # 原理图总览（示意）
    dual-cage-netlist.md         # 可导入 CAD 的逻辑网表
    sheets/                      # 分页面原理说明
  pcb/
    layout-sketch.svg            # 布局分区草图
    design-rules.md              # DRC / 叠层 / 覆铜规则
  bom/
    bom-revA.csv                 # Rev A 物料
  mechanical/
    cage-notes.md                # 笼子机械注意点
```

推荐落地流程：

1. 读 `docs/` 理解原理
2. 按 `dual-cage-netlist.md` 在 KiCad 建原理图符号与连接
3. 按实际笼子改板框与插座坐标
4. 按 `design-rules.md` 布铜
5. 按 `docs/06-safety-test.md` 上电
