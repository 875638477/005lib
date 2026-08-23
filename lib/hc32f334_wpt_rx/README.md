# HC32F334 无线充电接收端（20 / 30 / 50 W）

单线圈 PRx。第一轮整流走二极管，再切数字同步整流。输出 12 / 15 / 20 V 由 `WPT_POWER_W` 决定。

详细评估见 [`docs/HC32F334_无线充电_20_30_50W_方案评估.md`](../../docs/HC32F334_无线充电_20_30_50W_方案评估.md) 第 12～13 节。

发射端：[`../hc32f334_wpt_tx/`](../hc32f334_wpt_tx/)。

## 上板要点

- **LQFP64**（`HC32F334KATI` / `K8TI`）
- SR：PC6/PC7 + PA8/PA9；Buck：PA10/PA11
- ASK：PC4；Vrect：PA0；Vout：PA1；Iout：PA2
- CMP1 PA7 锁相；二极管阶段不要使能 SR PWM

## 主机自测

```
make -C tests test
```

会链上 TX 的 ASK 解调，确认收发编解码一致。
