from textwrap import dedent

import cocotb
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge, FallingEdge, ClockCycles

from dotmatrix import scanlines, assert_screen, printscreen
from paddle import Paddle

DEBOUNCEWIDTH = 2


async def wait_for_value(clk, signal, val, max_ticks):
    for i in range(max_ticks):
        await RisingEdge(clk)
        if signal == val:
            return signal
    raise TestFailure(f"{signal} did not reach value {val} within {max_ticks} clock ticks")


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

    dut.difficulty = 0
    lpaddle = Paddle(dut.player1_a, dut.player1_b)
    rpaddle = Paddle(dut.player2_a, dut.player2_b)

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

    print("Waiting for project to become active...")
    # wait for the project to become active
    await RisingEdge(dut.uut.mprj.pong_wrapper.active)

    print("Waiting for reset to go high...")
    await RisingEdge(dut.uut.mprj.pong_wrapper.pong0.reset)
    await FallingEdge(dut.uut.mprj.pong_wrapper.pong0.reset)
    print("Reset completed")

    assert dut.uut.mprj.pong_wrapper.pong0.game0.lpaddle == 0b00000000000011111111000000000000
    assert dut.uut.mprj.pong_wrapper.pong0.game0.rpaddle == 0b00000000000011111111000000000000

    print("Move paddles...")
    for _ in range(20):
        lpaddle.down()
        rpaddle.up()
        await ClockCycles(dut.clock, 2**DEBOUNCEWIDTH * 4)

    print("Wait until the screen is ready to draw the scanline that the ball is on (row %d)..." %
          (dut.uut.mprj.pong_wrapper.pong0.y.value.integer / 2))
    await wait_for_value(dut.clock, dut.uut.mprj.pong_wrapper.pong0.screen0.corrected_row, dut.uut.mprj.pong_wrapper.pong0.y.value.integer / 2, 3000)

    print("Capturing the contents of the next screen refresh...")
    screen = await scanlines(dut)
    printscreen(screen)
    assert_screen(dedent("""\
        0000000000000000
        1000000000000000
        1000000000000000
        1000000000000000
        1000000000000000
        0000000000000000
        0000000000000000
        0000000000000000
        0000000010000000
        0000000000000000
        0000000000000000
        0000000000000001
        0000000000000001
        0000000000000001
        0000000000000001
        0000000000000000
        """), screen)


@cocotb.test()
async def test_ball_movement(dut):
    clock = Clock(dut.clock, 31, units="ns")
    cocotb.fork(clock.start())

    dut.difficulty = 0
    print("Pressing start...")
    dut.start = 1
    await ClockCycles(dut.clock, 8)
    dut.start = 0

    dut.difficulty = 0xF
    cycles = int(2**16 / (127 * 15)) * 3 + 4
    print("Waiting %d clock cycles for the ball to move 1 pixel..." % cycles)
    await ClockCycles(dut.clock, cycles)
    x = dut.uut.mprj.pong_wrapper.pong0.x.value.integer
    y = dut.uut.mprj.pong_wrapper.pong0.y.value.integer
    dut.difficulty = 0  # prevent further ball movement while we capture the screen

    print(f"Ball now at: x={x} y={y}")
    print("Collecting next screen refresh...")
    printscreen(await scanlines(dut))

    # Since the direction is pseudo random based on the LFSR generator, anticipate all directions:
    assert (x, y) in (
        (16, 17), (16, 15), (15, 16), (17, 16), (15, 15), (17, 17), (15, 17), (17, 15))
