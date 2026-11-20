"""SYS-05..07: timing.

The F446 runs on its internal HSI oscillator, specified at +-1 % at 25 C
(datasheet DS10693). So 1000 ms may really be 990..1010 ms. We allow 2 %.
"""
import statistics
import time

PERIOD_S = 1.0
TOLERANCE_S = 0.020       # 2 %
JITTER_S = 0.005          # step-to-step variation
UART_LAG_S = 0.020        # LED edge -> its STEP line


def test_sys05_step_period_within_tolerance(board):
    board.wait_status(r"^STEP 6 ", timeout=6.5)
    t = [e.precise_t for e in board.rising_edges()]
    periods = [b - a for a, b in zip(t, t[1:])]
    assert len(periods) >= 5
    for p in periods:
        assert abs(p - PERIOD_S) <= TOLERANCE_S, f"period {p*1000:.1f} ms"
    assert statistics.pstdev(periods) <= JITTER_S, f"jitter {statistics.pstdev(periods)*1000:.2f} ms"


def test_sys06_uart_line_matches_led(board):
    for i in range(1, 5):
        st = board.wait_status(rf"^STEP {i} ", timeout=1.2)
        time.sleep(UART_LAG_S)                   # let the edge arrive, if it's late
        edge = [e for e in board.rising_edges() if e.pin == f"PC{i}"][-1]
        assert board.level[f"PC{i}"] == 1
        # LED and line come through different paths (GPIO vs USB), so either
        # can be seen first. They must be close together.
        assert abs(st.t - edge.t) <= UART_LAG_S, f"line and LED {(st.t-edge.t)*1000:.1f} ms apart"


def test_sys07_firmware_clock_steps_exactly_1000ms(board):
    a = board.wait_status(r"^STEP 1 ", timeout=1.2)
    b = board.wait_status(r"^STEP 2 ", timeout=1.2)
    assert b.fw_ms - a.fw_ms in (999, 1000, 1001)