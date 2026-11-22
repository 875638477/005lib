# Capture 阻容位号与 Value 快检

适用规则为 [RC-20261117.1](capture_rc_horizontal_ref_value_layout_spec.md)。快检为只读操作，可用于已有原理图。它按器件两只实际引脚的起点和热点判断横竖，分别查看原生可见的位号和完整 Value。对于常见的 R、C、PR、PC 位号和指定符号均会扫描，异形多脚符号、隐藏或有多项可见 Value 的实例需要人工确认。

## 在 Capture 中运行

打开目标 DSN 或 OPJ，在 **View → Command Window** 执行：

```tcl
source {D:/005lib/01script_file/capture_rc_text_check.tcl}
rc_text_check
```

报告路径会显示在命令窗口，默认输出到 Windows 临时目录。也可指定一个尚未存在的 TSV 路径，并传入本工程实核的网格，单位为 Capture 文档单位：

```tcl
rc_text_check {D:/my_project/rc_text_check.tsv} 5
```

示例中的 `5` 只适用于已核实为 5 文档单位的工程。报告按原理图、页名、实例 ID、位号和字段逐行定位。`axis_check` 是方向检查，`anchor_minus_lead_doc` 是显示锚点相对引脚线的有符号偏移。横件的偏移沿 Y 量，竖件沿 X 量。锚点或原生显示框用于定位，实际字形贴线在取得证据前保持 `UNKNOWN`。TSV 末行给出完成计数和读取错误数。

## 扫描后修正文字

在 Capture 中打开要整理的工作副本，然后执行：

```tcl
source {D:/005lib/01script_file/capture_rc_text_fix.tcl}
rc_text_fix
```

修正入口先读取器件位置和实际针脚，再判断横竖，并原生写入两项文字的方向和坐标。横件位号在左、Value 在右，两项横排。竖件位号在上、Value 在下，两项竖排。位置按网格搜索，贴近对应引脚轴，避开可建模的器件本体、引线、相邻文字和符号。脚本只写位号及 Value 的显示位置和方向。默认允许文字超出页边，报告的 `overflows_page` 列出相应字段。

`rc_text_fix` 读取 Capture 当前文档网格并修改活动设计。检查结果后可保存。也可指定网格、页名通配符、位号通配符和运行方式，例如：

```tcl
rc_text_fix 5 {P20.*} * preview
rc_text_fix 5 {P20.*} * apply-save
rc_text_fix 5 * * apply stay-on-page
```

`preview` 只生成计划。默认 `apply` 修改内存中的设计。`apply-save` 还会保存当前设计，适用于工作副本。`stay-on-page` 把页边纳入搜索约束。报告中的 `PLANNED` 表示该器件有编辑且两项均有候选位置，`PARTIAL_…` 表示只找到了部分文字的位置。其余待核原因逐器件列出。

修正器当前用已经原生打印校准的 Arial 9 显示框架（实际 LOGFONT `Arial -9 4 …`、100 文档单位/英寸、靠左、只显示值）与 Windows GDI 轮廓估算放置位置。其他字体、显示模式或无可用两脚引线坐标的实例保留待核。相邻障碍使用可读取区域的保守包络，个别未取到的图元会在报告里列出。应用后可用 `rc_text_check` 检查保存重开的方向，实际字形与邻域以原生显示复核。Python 优先使用本机 Codex bundled runtime，也可设置 `CAPTURE_RC_PYTHON` 指向已有 Python 3.11+ 的可执行文件。

## 核实实际字形到引线的间距

需要同次保存的 DSN、全字形原生打印测量报告和已批准的水平正例报告。使用 [原生打印校准流程](capture_native_print_text_and_canvas.md) 绑定实际打印字形、针热点、显示属性、字体与页面坐标。已有该流程生成的 `NATIVE_GLYPH_PROXIMITY.json` 时，可用学习库的比较脚本复核：

```powershell
python D:/005lib/01AIEDA/learning/scripts/capture_layout_tools/compare_capture_rc_text_ink.py `
  --scan D:/my_project/rc_text_check.tsv `
  --native-ink D:/my_project/NATIVE_GLYPH_PROXIMITY.json `
  --approved-ink D:/reference/NATIVE_GLYPH_PROXIMITY.json `
  --approved-ref DP_C5 `
  --output D:/my_project/rc_text_ink_result.json
```

比较使用同次保存的 DSN SHA、原生打印和读取证据、显示属性 ID、完整原生字形与实际热点。每项文字分别比较它在引线轴方向上的所属端，以及垂直间距是否超过对应的水平正例加一格。字体、对齐、文档单位、网格、对象或字形证据不匹配时，该项保持 `UNKNOWN`。比较器需要 Python 3.11 或以上。

正式验收继续用 [阻容规则检查器](../scripts/capture_layout_tools/check_rc_layout_contract.py) 对同版工程证据逐项核本机正式库与缓存、对齐、网格及完整图面避让，并按现行准则验紧凑度。工程副本、报告和原生打印文件留在工程目录。
