"""SYS-02..04: order, one LED at a time, heartbeat."""
from targets.base import LED_PINS


def test_sys02_runs_pc0_to_pc6_and_wraps(board):
    expected = [1, 2, 3, 4, 5, 6, 0, 1]
    for i in expected:
        st = board.wait_status(rf"^STEP {i} ", timeout=1.2)
        assert st.index == i
    # The reset itself makes PC0 go high once, so the order starts with PC0.
    order = [e.pin for e in board.rising_edges()]
    assert order == ["PC0"] + [f"PC{i}" for i in expected]


def test_sys03_never_two_leds_on_at_once(board):
    board.wait_status(r"^STEP 1 ", timeout=1.2)
    board.wait_status(r"^STEP 0 ", timeout=7.5)      # one full cycle incl. 6 -> 0
    on = {"PC0"}
    for e in board.edges_after_boot():               # replay every edge in order
        if e.pin not in LED_PINS:
            continue
        on.add(e.pin) if e.level else on.discard(e.pin)
        assert len(on) <= 1, f"two LEDs on at t={e.t:.4f}: {sorted(on)}"


def test_sys04_heartbeat_toggles_every_step(board):
    board.wait_status(r"^STEP 3 ", timeout=3.5)
    hb = [e.level for e in board.edges if e.pin == "PA5"]
    assert hb == [1, 0, 1]