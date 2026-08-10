# Sheet 05 — 可选 ORing（Rev B）

当需要故障隔离时，不要直并 `V12A/V12B`，改为：

```text
V12A ---- Rsense(opt) ---- MOSFET_A (N-ch 或控制器推荐拓扑) ---- VBUS_12V
                              ^
                         OR-CTRL_A (如 TPS2412)

V12B ---- Rsense(opt) ---- MOSFET_B ---- VBUS_12V
                              ^
                         OR-CTRL_B
```

要点：

1. FET 选逻辑电平驱动、极低 Rds(on)、足电流与散热（铜皮散热或散热片）
2. 控制器放在功率路径旁，采样线短
3. Active Droop 仍由 PSU 完成；ORing 解决的是反向电流/短路隔离
4. 参考 TI SLUA550：均流控制器 + ORing 的组合方案（电压轨不同需改参）

Rev A 打样成功并完成热测试后，再升级 Rev B。
