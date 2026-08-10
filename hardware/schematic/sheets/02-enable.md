# Sheet 02 — 使能与 Present

## 槽 A

```text
SB_A ---- R1 22k ----+---- PRE_A ---- J1.36
                     |
                    TP1

J1.33 EN_A# ----+---- SW1 ---- SGND
                |
               TP2
                |
            (可选开漏 MOSFET from MCU)
```

## 槽 B

```text
SB_B ---- R2 22k ----+---- PRE_B ---- J2.36
                     |
                    TP3

J2.33 EN_B# ----+---- SW2 ---- SGND
                |
               TP4
```

## 全局使能（可选）

```text
EN_A# ----+---- JP1 ---- EN_GLOBAL# ---- SW_GLOBAL ---- SGND
EN_B# ----+
```

注意：`EN#` 是信号电流，开关额定不必很大；但触点应可靠。
