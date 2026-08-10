# Sheet 03 — 状态指示与告警

```text
                    +-- R10 -- LED_OK_A --> SB_A 或 3V3
PSOK_A (J1.35) -----+
                    +-- 分压 --> MCU_IN_A (可选)

                    +-- R11 -- LED_OK_B --> SB_B 或 3V3
PSOK_B (J2.35) -----+

ALARM_A (J1.38) ---- R12 -- LED_AL_A
ALARM_B (J2.38) ---- R13 -- LED_AL_B
```

实施前用万用表/示波器确认：

1. PSOK 在 OK 时是高还是低
2. 是否有能力灌/拉电流
3. 再决定 LED 是高边还是低边点亮

预留 0Ω 跳线改变 LED 参考电源（SB vs 3V3）。
