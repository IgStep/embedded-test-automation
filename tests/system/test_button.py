"""SYS-13..14: blue button B1 (pressed by the Pi on real hardware)."""
import time


def test_sys13_press_reverses_direction(board):
    board.press_button(0.1)
    st = board.wait_status(r"^BTN ", timeout=0.5)
    assert st.dir == "-"
    assert board.wait_status(r"^STEP ", timeout=1.2).index == 6


def test_sys14_short_glitch_is_ignored(board):
    board.press_button(0.005)          # 5 ms, debounce is 30 ms
    time.sleep(0.3)
    assert not [l for l in board.lines if l.text.startswith("BTN")]
    assert board.wait_status(r"^STEP ", timeout=1.2).index == 1