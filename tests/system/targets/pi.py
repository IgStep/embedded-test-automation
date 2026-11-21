"""
Pi target: the real NUCLEO-F446RE, seen through a Raspberry Pi.

  * LEDs:   Pi GPIO inputs, edges timestamped by the kernel (lgpio)
  * Button: Pi GPIO12 pulls PC13 low (= pressed), or floats (= released)
  * UART:   /dev/ttyACM0 (ST-LINK virtual COM port), via pyserial
  * Reset/flash: st-flash over the same USB cable
Wiring: docs/WIRING.md
"""
import subprocess
import threading
import time

import lgpio
import serial

from .base import Target

# Pi GPIO (BCM number) for each Nucleo pin. Must match the wiring.
PIN_MAP = {
    "PC0": 5, "PC1": 6, "PC2": 13, "PC3": 19,
    "PC4": 26, "PC5": 16, "PC6": 20, "PA5": 21,
}
BUTTON_GPIO = 12


class PiTarget(Target):
    def __init__(self, port="/dev/ttyACM0", chip=0):
        super().__init__()
        self.h = lgpio.gpiochip_open(chip)
        self._gpio_to_pin = {g: p for p, g in PIN_MAP.items()}
        self._callbacks = []
        for pin, gpio in PIN_MAP.items():
            lgpio.gpio_claim_alert(self.h, gpio, lgpio.BOTH_EDGES)
            self._callbacks.append(lgpio.callback(self.h, gpio, lgpio.BOTH_EDGES, self._on_edge))
        lgpio.gpio_claim_input(self.h, BUTTON_GPIO)          # released
        self.ser = serial.Serial(port, 115200, timeout=0.05)
        self._stop = False
        threading.Thread(target=self._read_uart, daemon=True).start()

    def _on_edge(self, chip, gpio, level, tick_ns):
        # tick_ns is the kernel's timestamp of the edge: much more precise
        # than the moment Python got around to calling us. We use it for
        # interval measurements (period, jitter).
        if level in (0, 1):                     # 2 = watchdog timeout, ignore
            self._add_edge(self._gpio_to_pin[gpio], level, hw_t=tick_ns / 1e9)

    def _read_uart(self):
        while not self._stop:
            raw = self.ser.readline()           # returns b"" after 50 ms if nothing came
            if raw:
                self._add_line(raw.decode(errors="replace").strip())

    def flash(self, binary):
        subprocess.run(["st-flash", "--reset", "write", str(binary), "0x08000000"], check=True)

    def reset(self):
        self.button(False)
        self._clear()
        subprocess.run(["st-flash", "reset"], check=True, capture_output=True)
        self._wait_boot(timeout=5)
        for pin, gpio in PIN_MAP.items():        # real state after boot
            self.level[pin] = lgpio.gpio_read(self.h, gpio)

    def send(self, text):
        self.ser.write(text.encode())

    def button(self, pressed):
        if pressed:
            lgpio.gpio_claim_output(self.h, BUTTON_GPIO, 0)   # drive low = press
        else:
            lgpio.gpio_claim_input(self.h, BUTTON_GPIO)       # float = release (never drive high!)

    def close(self):
        self._stop = True
        self.button(False)
        for cb in self._callbacks:
            cb.cancel()
        self.ser.close()
        lgpio.gpiochip_close(self.h)