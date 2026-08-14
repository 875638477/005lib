#!/usr/bin/env python3
"""TMN1APSU / 华为同门电源：PMBus 风格风扇探测与开机后降速。

默认只读。写转速前必须先 --probe 看到合理的厂家/型号/转速。
写入的 FAN_COMMAND 通常掉电丢失，脚本不会发送 STORE_*。
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import time
from typing import Optional

CMD_CAPABILITY = 0x19
CMD_FAN_CONFIG = 0x3A
CMD_FAN_COMMAND = 0x3B
CMD_STATUS_FANS = 0x81
CMD_READ_TEMP_1 = 0x8D
CMD_READ_TEMP_2 = 0x8E
CMD_READ_FAN_SPEED = 0x90
CMD_PMBUS_REVISION = 0x98
CMD_MFR_ID = 0x99
CMD_MFR_MODEL = 0x9A
CMD_MFR_REVISION = 0x9B

HUAWEI_7BIT = list(range(0x58, 0x60))
PMBUS_COMMON_7BIT = [0x58, 0x59, 0x5A, 0x5B, 0x40, 0x41, 0x42, 0x43]


def linear11_decode(raw: int) -> float:
    n = (raw >> 11) & 0x1F
    if n & 0x10:
        n -= 0x20
    y = raw & 0x7FF
    if y & 0x400:
        y -= 0x800
    return float(y) * (2.0**n)


def linear11_encode_int(value: int) -> int:
    if value < -1024 or value > 1023:
        raise ValueError("Linear11 mantissa out of range")
    return value & 0x7FF


def pec_crc8(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


class I2CError(RuntimeError):
    pass


class I2CBus:
    def __init__(self, bus: int, pec: bool) -> None:
        self.bus = bus
        self.pec = pec
        self._smbus = None
        self._use_tools = False
        self._open()

    def _open(self) -> None:
        try:
            from smbus2 import SMBus  # type: ignore

            self._smbus = SMBus(self.bus)
            if self.pec:
                try:
                    self._smbus.pec = True
                except Exception:
                    pass
            return
        except Exception:
            self._smbus = None
        if not self._have_i2c_tools():
            raise I2CError(
                "既没有 smbus2，也没有 i2cget/i2cset。"
                "请安装 python3-smbus2 或 i2c-tools。"
            )
        self._use_tools = True

    @staticmethod
    def _have_i2c_tools() -> bool:
        try:
            subprocess.run(
                ["i2cget", "-V"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )
            return True
        except FileNotFoundError:
            return False

    def close(self) -> None:
        if self._smbus is not None:
            self._smbus.close()

    def _tool(self, args: list[str]) -> str:
        cmd = args[:]
        if self.pec and cmd[0] in ("i2cget", "i2cset"):
            cmd.insert(1, "-p")
        try:
            out = subprocess.run(
                cmd,
                check=True,
                capture_output=True,
                text=True,
            )
        except subprocess.CalledProcessError as exc:
            raise I2CError(exc.stderr.strip() or str(exc)) from exc
        return out.stdout.strip()

    def read_byte(self, addr: int, cmd: int) -> int:
        if self._smbus is not None:
            return int(self._smbus.read_byte_data(addr, cmd))
        text = self._tool(["i2cget", "-y", str(self.bus), hex(addr), hex(cmd), "b"])
        return int(text, 0)

    def read_word(self, addr: int, cmd: int) -> int:
        if self._smbus is not None:
            return int(self._smbus.read_word_data(addr, cmd))
        text = self._tool(["i2cget", "-y", str(self.bus), hex(addr), hex(cmd), "w"])
        return int(text, 0)

    def write_word(self, addr: int, cmd: int, value: int) -> None:
        if self._smbus is not None:
            self._smbus.write_word_data(addr, cmd, value)
            return
        self._tool(
            [
                "i2cset",
                "-y",
                str(self.bus),
                hex(addr),
                hex(cmd),
                hex(value & 0xFF),
                hex((value >> 8) & 0xFF),
                "w",
            ]
        )

    def read_block(self, addr: int, cmd: int) -> bytes:
        if self._smbus is not None:
            raw = self._smbus.read_i2c_block_data(addr, cmd, 16)
            if not raw:
                return b""
            n = raw[0]
            return bytes(raw[1 : 1 + n])
        try:
            text = self._tool(
                ["i2cget", "-y", str(self.bus), hex(addr), hex(cmd), "i"]
            )
        except I2CError:
            return b""
        parts = [int(p, 0) for p in text.replace(",", " ").split() if p]
        if not parts:
            return b""
        n = parts[0]
        return bytes(parts[1 : 1 + n])


def decode_ascii(data: bytes) -> str:
    return "".join(chr(b) if 32 <= b < 127 else "." for b in data).strip()


def probe_one(bus: I2CBus, addr: int) -> dict:
    info: dict = {"addr": addr}
    try:
        info["capability"] = bus.read_byte(addr, CMD_CAPABILITY)
    except I2CError as exc:
        info["error"] = str(exc)
        return info
    for key, cmd, kind in (
        ("pmbus_rev", CMD_PMBUS_REVISION, "byte"),
        ("fan_config", CMD_FAN_CONFIG, "byte"),
        ("fan_command", CMD_FAN_COMMAND, "word"),
        ("status_fans", CMD_STATUS_FANS, "byte"),
        ("fan_speed_raw", CMD_READ_FAN_SPEED, "word"),
        ("temp1_raw", CMD_READ_TEMP_1, "word"),
        ("temp2_raw", CMD_READ_TEMP_2, "word"),
    ):
        try:
            info[key] = (
                bus.read_byte(addr, cmd) if kind == "byte" else bus.read_word(addr, cmd)
            )
        except I2CError:
            info[key] = None
    for key, cmd in (
        ("mfr_id", CMD_MFR_ID),
        ("mfr_model", CMD_MFR_MODEL),
        ("mfr_revision", CMD_MFR_REVISION),
    ):
        try:
            info[key] = decode_ascii(bus.read_block(addr, cmd))
        except I2CError:
            info[key] = ""
    if info.get("fan_speed_raw") is not None:
        info["fan_speed_rpm"] = linear11_decode(info["fan_speed_raw"])
    if info.get("fan_command") is not None:
        info["fan_command_value"] = linear11_decode(info["fan_command"])
    if info.get("temp1_raw") is not None:
        info["temp1_c"] = linear11_decode(info["temp1_raw"])
    if info.get("temp2_raw") is not None:
        info["temp2_c"] = linear11_decode(info["temp2_raw"])
    return info


def print_info(info: dict) -> None:
    addr = info["addr"]
    if "error" in info and len(info) <= 2:
        print(f"  0x{addr:02X}: 无应答 ({info['error']})")
        return
    print(f"  0x{addr:02X}:")
    if info.get("mfr_id") or info.get("mfr_model"):
        print(f"    厂家/型号: {info.get('mfr_id', '')} {info.get('mfr_model', '')}")
        print(f"    版本: {info.get('mfr_revision', '')}")
    if info.get("capability") is not None:
        print(f"    CAPABILITY(0x19): 0x{info['capability']:02X}")
    if info.get("fan_config") is not None:
        cfg = info["fan_config"]
        mode = "RPM" if (cfg >> 6) & 1 else "duty%"
        print(f"    FAN_CONFIG(0x3A): 0x{cfg:02X}  (命令格式 {mode})")
    if info.get("fan_command") is not None:
        print(
            f"    FAN_COMMAND(0x3B): 0x{info['fan_command']:04X}"
            f"  ≈ {info.get('fan_command_value')}"
        )
    if info.get("fan_speed_raw") is not None:
        print(
            f"    READ_FAN_SPEED(0x90): 0x{info['fan_speed_raw']:04X}"
            f"  ≈ {info.get('fan_speed_rpm'):.0f} RPM"
        )
    if info.get("status_fans") is not None:
        print(f"    STATUS_FANS(0x81): 0x{info['status_fans']:02X}")
    if info.get("temp1_raw") is not None:
        print(f"    TEMP1: {info.get('temp1_c'):.1f} °C")
    if info.get("temp2_raw") is not None:
        print(f"    TEMP2: {info.get('temp2_c'):.1f} °C")


def run_probe(bus_id: int, addrs: list[int], pec: Optional[bool]) -> None:
    pec_modes = [True, False] if pec is None else [pec]
    found = False
    for use_pec in pec_modes:
        print(f"\n== bus {bus_id}  PEC={'on' if use_pec else 'off'} ==")
        bus = I2CBus(bus_id, use_pec)
        try:
            for addr in addrs:
                info = probe_one(bus, addr)
                if "error" in info and len(info) <= 2:
                    continue
                found = True
                print_info(info)
        finally:
            bus.close()
    if not found:
        print("没有读到可用从设备。检查 SCL/SDA/3.3V 上拉、地址绑线和总线号。")


def run_set_duty(bus_id: int, addr: int, duty: int, pec: bool) -> None:
    if duty < 0 or duty > 100:
        raise SystemExit("占空比必须是 0–100")
    print(
        "警告：降低电源风扇转速可能过热。"
        "空载小步试验；壳体发烫或 STATUS_FANS 置位就立刻升回去。"
    )
    bus = I2CBus(bus_id, pec)
    try:
        before = probe_one(bus, addr)
        print("写入前：")
        print_info(before)
        if "error" in before and before.get("fan_config") is None:
            raise SystemExit("该地址读失败，拒绝写入。")
        cfg = before.get("fan_config")
        if cfg is not None and (cfg >> 6) & 1:
            raise SystemExit(
                "FAN_CONFIG bit6=1，当前是 RPM 模式，拒绝按百分比写入。"
            )
        word = linear11_encode_int(duty)
        bus.write_word(addr, CMD_FAN_COMMAND, word)
        time.sleep(1.0)
        after = probe_one(bus, addr)
        print(f"已写 FAN_COMMAND={duty}% (0x{word:04X})，回读：")
        print_info(after)
        print("该设置通常掉电丢失。需要的话放到开机脚本里每次执行。")
    finally:
        bus.close()


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--bus", type=int, default=1, help="i2c 总线号，默认 1")
    p.add_argument("--addr", type=lambda s: int(s, 0), help="7-bit 从地址，例如 0x58")
    p.add_argument("--probe", action="store_true", help="只读扫描/探测")
    p.add_argument("--set-duty", type=int, metavar="PCT", help="写入风扇占空比 0-100")
    p.add_argument("--pec", action="store_true", help="强制使用 PEC")
    p.add_argument("--no-pec", action="store_true", help="强制不使用 PEC")
    return p.parse_args()


def main() -> int:
    args = parse_args()
    pec: Optional[bool]
    if args.pec and args.no_pec:
        print("--pec 和 --no-pec 不能同时用", file=sys.stderr)
        return 2
    if args.pec:
        pec = True
    elif args.no_pec:
        pec = False
    else:
        pec = None

    addrs = [args.addr] if args.addr is not None else sorted(set(HUAWEI_7BIT + PMBUS_COMMON_7BIT))

    if args.set_duty is not None:
        if args.addr is None:
            print("写转速必须指定 --addr", file=sys.stderr)
            return 2
        if pec is None:
            pec = True
        run_set_duty(args.bus, args.addr, args.set_duty, pec)
        return 0

    run_probe(args.bus, addrs, pec)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
