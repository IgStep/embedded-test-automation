"""SYS-01: boot. Test IDs map to docs/TEST_PLAN.md."""


def test_sys01_boot_prints_version_and_lights_pc0(board):
    from targets.base import Status
    assert board.lines[0].text == "BOOT led_seq 1.0.0"
    st = Status.parse(board.lines[1])            # the line right after BOOT
    assert (st.index, st.dir, st.period, st.paused) == (0, "+", 1000, False)
    assert board.leds_on() == ["PC0"]
