"""SYS-08..12: UART commands."""
import time


def test_sys08_r_reverses(board):
    board.send("r")
    st = board.wait_status(r"^OK ", timeout=0.5)
    assert (st.index, st.dir) == (0, "-")
    assert board.wait_status(r"^STEP ", timeout=1.2).index == 6
    assert board.wait_status(r"^STEP ", timeout=1.2).index == 5


def test_sys09_f_halves_period_with_lower_limit(board):
    periods = []
    for _ in range(5):
        board.send("f")
        periods.append(board.wait_status(r"^OK ", timeout=0.5).period)
    assert periods == [500, 250, 125, 125, 125]
    a = board.wait_status(r"^STEP ", timeout=0.3)
    b = board.wait_status(r"^STEP ", timeout=0.3)
    assert abs((b.t - a.t) - 0.125) < 0.02


def test_sys10_s_doubles_period_with_upper_limit(board):
    periods = []
    for _ in range(3):
        board.send("s")
        periods.append(board.wait_status(r"^OK ", timeout=0.5).period)
    assert periods == [2000, 2000, 2000]


def test_sys11_p_pauses_and_resumes(board):
    board.send("p")
    assert board.wait_status(r"^OK ", timeout=0.5).paused
    time.sleep(2.5)
    assert not [l for l in board.lines if l.text.startswith("STEP 1")]
    assert board.leds_on() == ["PC0"]
    board.send("p")
    assert not board.wait_status(r"^OK ", timeout=0.5).paused
    t_resume = time.monotonic()
    board.wait_status(r"^STEP 1 ", timeout=1.2)
    assert time.monotonic() - t_resume >= 0.95      # full period after resume


def test_sys12_question_mark_reports_and_junk_gives_err(board):
    board.send("?")
    st = board.wait_status(r"^OK ", timeout=0.5)
    assert (st.index, st.period) == (0, 1000)
    board.send("x")
    board.wait_line(r"^ERR$", timeout=0.5)
    board.send("\r\n")                               # Enter is ignored
    time.sleep(0.2)
    assert [l.text for l in board.lines].count("ERR") == 1