from textwrap import dedent

import cocotb
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge, FallingEdge, ClockCycles
from encoder import Encoder

clocks_per_phase = 10


async def wait_for_value(clk, signal, val, max_ticks):
    for i in range(max_ticks):
        await RisingEdge(clk)
        if (signal == val):
            return signal
    raise TestFailure(f"{signal} did not reach value {val} within {max_ticks} clock ticks")


async def scanlines(dut):
    """Collects the next full screen of 16 horizontal scanlines.

    This waits for the screen refresh circuit to reach the end of the currently
    drawing screen, return to the top left of the screen and then captures the
    16 scanlines.

    The screen contents are returned as a list of 16 integers.
    """
    lines = [0] * 16

    await FallingEdge(dut.RSDI) # wait for start of a new screen...

    for row in range(16):
        await RisingEdge(dut.RCLK)  # wait for the first row to be ready...
        for col in range(16):
            lines[row + (-1 if (row & 1) else 1)] |= dut.CSDI.value << col
            if col < 15:
                await RisingEdge(dut.CCLK)

    return lines


def printscreen(scanlines) -> None:
    for scanline in scanlines:
        print(bin(scanline)[2:].rjust(16, '0'))


def assert_screen(expected: str, scanlines) -> None:
    for i, line in enumerate(expected.splitlines()):
        if (line != bin(scanlines[i])[2:].rjust(16, '0')):
            error = "Screen mismatch\n"
            for i, line in enumerate(expected.splitlines()):
                error += (line + "        " + bin(scanlines[i])[2:].rjust(16, '0') + "\n")
            error += "    Expected                 Actual\n"
            raise TestFailure(error)


@cocotb.test()
async def test_start(dut):
    clock = Clock(dut.clock, 25, units="ns")
    cocotb.fork(clock.start())
    print("Test has started the clock")
    
    dut.RSTB <= 0
    dut.power1 <= 0
    dut.power2 <= 0
    dut.power3 <= 0
    dut.power4 <= 0

    await ClockCycles(dut.clock, 8)
    dut.power1 <= 1
    await ClockCycles(dut.clock, 8)
    dut.power2 <= 1
    await ClockCycles(dut.clock, 8)
    dut.power3 <= 1
    await ClockCycles(dut.clock, 8)
    dut.power4 <= 1

    print("Power lines are all high...")

    await ClockCycles(dut.clock, 80)
    dut.RSTB <= 1

    dut.start = 0
    dut.player1_a <= 0
    dut.player1_b <= 0
    dut.player2_a <= 0
    dut.player2_b <= 0

    print("Waiting for project to become active...")
    # wait for the project to become active
    await RisingEdge(dut.uut.mprj.pong_wrapper.active)

    print("Waiting for reset to go high...")
    await RisingEdge(dut.uut.mprj.pong_wrapper.pong0.reset)
    await FallingEdge(dut.uut.mprj.pong_wrapper.pong0.reset)
    print("Reset completed")

    assert(dut.uut.mprj.pong_wrapper.pong0.game0.lpaddle == 0b0000001111000000)
    assert(dut.uut.mprj.pong_wrapper.pong0.game0.rpaddle == 0b0000001111000000)

    print("Wait until the screen is ready to draw the row that the ball is on (row %d)..." %
        dut.uut.mprj.pong_wrapper.pong0.y.value)
    await wait_for_value(dut.clock, dut.uut.mprj.pong_wrapper.pong0.screen0.corrected_row, dut.uut.mprj.pong_wrapper.pong0.y.value, 1000)

    print("Capturing the contents of the next screen refresh...")
    screen = await scanlines(dut)
    assert_screen(dedent("""\
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        1000000000000001
        1000000000000001
        1000000100000001
        1000000000000001
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        0000000000000000"""), screen)
