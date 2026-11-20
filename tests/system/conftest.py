"""
pytest configuration for the system tests.

  pytest tests/system --target host            # no hardware, on any PC
  pytest tests/system --target pi --fw build-fw/led_seq.bin   # HIL on the Pi

The 'board' fixture gives every test a freshly reset board.
"""
import pytest


def pytest_addoption(parser):
    parser.addoption("--target", default="host", choices=["host", "pi"],
                     help="host = PC simulation, pi = real Nucleo via Raspberry Pi")
    parser.addoption("--fw", default=None, help="firmware .bin to flash first (pi only)")
    parser.addoption("--port", default="/dev/ttyACM0", help="Nucleo serial port (pi only)")


@pytest.fixture(scope="session")
def target(request, tmp_path_factory):
    kind = request.config.getoption("--target")
    if kind == "host":
        from targets.host import HostTarget, build
        t = HostTarget(build(tmp_path_factory.mktemp("host")))
    else:
        from targets.pi import PiTarget
        t = PiTarget(port=request.config.getoption("--port"))
        fw = request.config.getoption("--fw")
        if fw:
            t.flash(fw)
    yield t
    t.close()


@pytest.fixture
def board(target):
    target.reset()
    return target


def pytest_report_header(config):
    return f"target: {config.getoption('--target')}"
