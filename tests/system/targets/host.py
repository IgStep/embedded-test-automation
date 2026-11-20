"""Host target: runs the firmware logic as a normal PC program (host_sim)."""
import os
import subprocess
import threading
from pathlib import Path

from .base import Target

ROOT = Path(__file__).resolve().parents[3]
APP = ROOT / "nucleo_led" / "App"


def build(out_dir):
    out = Path(out_dir) / "host_sim"
    src = [Path(__file__).parent / "host_sim.c"] + [APP / f for f in ("app.c", "led_seq.c", "button.c")]
    subprocess.run(["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", f"-I{APP}",
                    *map(str, src), "-o", str(out)], check=True)
    return out


class HostTarget(Target):
    def __init__(self, exe):
        super().__init__()
        self.exe = exe
        self.proc = None

    def reset(self):
        self.close()
        self._clear()
        for p in self.level:
            self.level[p] = 0
        self.proc = subprocess.Popen([str(self.exe)], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
        threading.Thread(target=self._read_uart, args=(self.proc,), daemon=True).start()
        threading.Thread(target=self._read_pins, args=(self.proc,), daemon=True).start()
        self._wait_boot(timeout=2)

    def _read_uart(self, proc):
        for raw in proc.stdout:
            self._add_line(raw.decode(errors="replace").strip())

    def _read_pins(self, proc):
        for raw in proc.stderr:
            parts = raw.decode().split()
            if len(parts) == 3 and parts[0] == "PIN":
                self._add_edge(parts[1], int(parts[2]))

    def send(self, text):
        self.proc.stdin.write(text.encode())

    def button(self, pressed):
        self.proc.stdin.write(b"\x01" if pressed else b"\x02")

    def close(self):
        if self.proc:
            self.proc.kill()
            self.proc.wait()
            self.proc = None