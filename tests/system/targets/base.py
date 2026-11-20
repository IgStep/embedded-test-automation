"""
Common part of every test target (real board on a Pi, or host simulation).

A target collects two streams, each item with a timestamp from
time.monotonic() (seconds):
  * lines  - text lines received on the UART
  * edges  - pin changes:  Edge(t, "PC3", 1)
Tests only use the methods below, so the same test file runs on any target.
"""
import re
import threading
import time
from dataclasses import dataclass

LED_PINS = [f"PC{i}" for i in range(7)]
HEARTBEAT = "PA5"
STEP_RE = re.compile(r"(STEP|OK|BTN) (\d) DIR ([+-]) PERIOD (\d+) T (\d+)( PAUSED)?")


@dataclass
class Line:
    t: float
    text: str


@dataclass
class Edge:
    t: float             # time.monotonic() when we saw it (shared with lines)
    pin: str
    level: int
    hw_t: float = None   # precise hardware timestamp in s, if the target has one

    @property
    def precise_t(self):
        """Best timestamp for measuring intervals between edges."""
        return self.hw_t if self.hw_t is not None else self.t


@dataclass
class Status:
    """A parsed 'STEP 3 DIR + PERIOD 1000 T 4000' line."""
    t: float
    tag: str
    index: int
    dir: str
    period: int
    fw_ms: int
    paused: bool

    @classmethod
    def parse(cls, line):
        m = STEP_RE.fullmatch(line.text)
        if not m:
            raise ValueError(f"not a status line: {line.text!r}")
        return cls(line.t, m[1], int(m[2]), m[3], int(m[4]), int(m[5]), bool(m[6]))


class Target:
    def __init__(self):
        self._cond = threading.Condition()
        self.lines = []          # every Line received since the last reset
        self.edges = []          # every Edge seen since the last reset
        self._read_pos = 0       # how far wait_line() has consumed self.lines
        self.level = {p: 0 for p in LED_PINS + [HEARTBEAT]}

    # ---- called by the subclasses' reader threads ----------------------
    def _add_line(self, text):
        with self._cond:
            self.lines.append(Line(time.monotonic(), text))
            self._cond.notify_all()

    def _add_edge(self, pin, level, hw_t=None):
        with self._cond:
            self.level[pin] = level
            self.edges.append(Edge(time.monotonic(), pin, level, hw_t))
            self._cond.notify_all()

    def _clear(self):
        with self._cond:
            self.lines.clear()
            self.edges.clear()
            self._read_pos = 0

    # ---- what tests use -------------------------------------------------
    def wait_line(self, pattern, timeout=2.0):
        """Wait for the next UART line matching the regex 'pattern'.
        Lines before it are skipped. Raises AssertionError on timeout."""
        rx = re.compile(pattern)
        deadline = time.monotonic() + timeout
        with self._cond:
            while True:
                while self._read_pos < len(self.lines):
                    line = self.lines[self._read_pos]
                    self._read_pos += 1
                    if rx.search(line.text):
                        return line
                left = deadline - time.monotonic()
                if left <= 0:
                    seen = [l.text for l in self.lines[-5:]]
                    raise AssertionError(f"no line matching {pattern!r} within {timeout}s, last lines: {seen}")
                self._cond.wait(left)

    def _wait_boot(self, timeout):
        """After a reset: wait for BOOT and the first STEP 0 line.
        self.boot_line is that STEP 0 line; at that moment only PC0 is on."""
        self.wait_line(r"^BOOT ", timeout)
        self.boot_line = self.wait_line(r"^STEP 0 ", timeout=0.5)

    def edges_after_boot(self):
        return [e for e in self.edges if e.t > self.boot_line.t]

    def wait_status(self, pattern, timeout=2.0):
        return Status.parse(self.wait_line(pattern, timeout))

    def leds_on(self):
        return [p for p in LED_PINS if self.level[p]]

    def rising_edges(self, pins=LED_PINS):
        return [e for e in self.edges if e.pin in pins and e.level == 1]

    # ---- implemented per target ----------------------------------------
    def reset(self):
        """Restart the firmware and wait until it printed BOOT."""
        raise NotImplementedError

    def send(self, text):
        raise NotImplementedError

    def button(self, pressed):
        raise NotImplementedError

    def press_button(self, seconds):
        self.button(True)
        time.sleep(seconds)
        self.button(False)

    def close(self):
        pass