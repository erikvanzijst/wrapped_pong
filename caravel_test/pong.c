/*
 * SPDX-FileCopyrightText: 2020 Efabless Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "verilog/dv/caravel/defs.h"
/*
    IO Test:
        - Configures MPRJ lower 8-IO pins as outputs
        - Observes counter value through the MPRJ lower 8 IO pins (in the testbench)
*/

void main()
{
    /* 
    IO Control Registers
    | DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
    | 3-bits | 1-bit | 1-bit | 1-bit  | 1-bit  | 1-bit | 1-bit   | 1-bit   | 1-bit | 1-bit | 1-bit   |

    Output: 0000_0110_0000_1110  (0x1808) = GPIO_MODE_USER_STD_OUTPUT
    | DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
    | 110    | 0     | 0     | 0      | 0      | 0     | 0       | 1       | 0     | 0     | 0       |
    
     
    Input: 0000_0001_0000_1111 (0x0402) = GPIO_MODE_USER_STD_INPUT_NOPULL
    | DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
    | 001    | 0     | 0     | 0      | 0      | 0     | 0       | 0       | 0     | 1     | 0       |

    */

    // Start button:
    reg_mprj_io_8 =   GPIO_MODE_USER_STD_INPUT_NOPULL;

    // Player 1 rotary encoder:
    reg_mprj_io_9 =   GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_10 =  GPIO_MODE_USER_STD_INPUT_NOPULL;

    // Player 2 rotary encoder:
    reg_mprj_io_11 =  GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_12 =  GPIO_MODE_USER_STD_INPUT_NOPULL;

    // 7-segment scoreboards:
    reg_mprj_io_13 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_14 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_15 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_16 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_17 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_18 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_19 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_20 =  GPIO_MODE_USER_STD_OUTPUT;

    // Matrix display:
    reg_mprj_io_21 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_22 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_23 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_24 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_25 =  GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_26 =  GPIO_MODE_USER_STD_OUTPUT;

    // VGA port:
    reg_mprj_io_27 =  GPIO_MODE_USER_STD_OUTPUT;    // hsync
    reg_mprj_io_28 =  GPIO_MODE_USER_STD_OUTPUT;    // vsync
    reg_mprj_io_29 =  GPIO_MODE_USER_STD_OUTPUT;    // r0
    reg_mprj_io_30 =  GPIO_MODE_USER_STD_OUTPUT;    // r1
    reg_mprj_io_31 =  GPIO_MODE_USER_STD_OUTPUT;    // g0
    reg_mprj_io_32 =  GPIO_MODE_USER_STD_OUTPUT;    // g1
    reg_mprj_io_33 =  GPIO_MODE_USER_STD_OUTPUT;    // b0
    reg_mprj_io_34 =  GPIO_MODE_USER_STD_OUTPUT;    // b1

    // Difficulty
    reg_mprj_io_35 =  GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_36 =  GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_37 =  GPIO_MODE_USER_STD_INPUT_NOPULL;

    /* Apply configuration */
    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);

    // Enable SPI master
    // SPI master configuration bits:
    // bits 7-0:	Clock prescaler value (default 2)
    // bit  8:		MSB/LSB first (0 = MSB first, 1 = LSB first)
    // bit  9:		CSB sense (0 = inverted, 1 = noninverted)
    // bit 10:		SCK sense (0 = noninverted, 1 = inverted)
    // bit 11:		mode (0 = read/write opposite edges, 1 = same edges)
    // bit 12:		stream (1 = CSB ends transmission)
    // bit 13:		enable (1 = enabled)
    // bit 14:		IRQ enable (1 = enabled)
    // bit 15:		Connect to housekeeping SPI (1 = connected)
    reg_spimaster_config = 0xa002;	// Enable, prescaler = 2,
					                // connect to housekeeping SPI

    /*
     * PLL Configuration
     *
     * See Caravel datasheet and rough clock layout:
     * - https://github.com/lakshmi-sathi/avsdpll1v8_caravel/blob/master/doc/caravel_datasheet.pdf
     * - https://gist.github.com/kbeckmann/da4bc07d7ddfe854074e74822a10cc9e
     *
     * Since the Pong circuit's VGA module needs a 31.5MHz clock while the
     * external oscillator is 10MHz, use the PLL to multiply the clock.
     *
     * The PLL is configured through values for the output divider and
     * feedback divider. Suitable values can be found running @kbeckmann's
     * calculator: https://github.com/kbeckmann/caravel-pll-calculator
     *
     * The closest the PLL can get on an external 10MHz clock is 31.667MHz:
     *
     * $ python3 caravel_pll.py generate --allow-deviation --clkin 10 -o 31.5
     *  PLL Parameters:
     *
     *  clkin:    10.00 MHz
     *  clkout:   31.67 MHz
     *  clkout90: 31.67 MHz
     *
     *  PLL Feedback Divider: 19
     *  PLL Output Divider 1: 6
     *  PLL Output Divider 2: 6
     *
     *  Register 0x11: 0x36
     *  Register 0x12: 0x13
     *
     * Caravel only passes the PLL's primary output clock to the user space
     * area through `wb_clk_i`. The wrapper then divides it down to 12MHz to
     * run the slower game modules.
     */

    // Enable the PLL
    reg_spimaster_config = 0xb002;	// Apply stream mode
    reg_spimaster_data = 0x80;		// Write 0x80 (write mode)
    reg_spimaster_data = 0x08;		// Write 0x18 (start address)
    reg_spimaster_data = 0x01;		// Write 0x01 to PLL enable, no DCO mode
    reg_spimaster_config = 0xa102;	// Release CSB (ends stream mode)

    // Turn off PLL bypass (run the system from the PLL output clock)
    reg_spimaster_config = 0xb002;	// Apply stream mode
    reg_spimaster_data = 0x80;		// Write 0x80 (write mode)
    reg_spimaster_data = 0x09;		// Write 0x09 (start address)
    reg_spimaster_data = 0x00;		// Write 0x00 to clock from PLL (no bypass)
    reg_spimaster_config = 0xa102;	// Release CSB (ends stream mode)

    // Configure PLL output divider
    reg_spimaster_config = 0xb002;	// Apply stream mode
    reg_spimaster_data = 0x80;		// Write 0x80 (write mode)
    reg_spimaster_data = 0x11;		// Write 0x11 (start address) PLL output divider
    reg_spimaster_data = 0x36;		// Write 0x06 to PLL output divider 1 and 2
    reg_spimaster_config = 0xa102;	// Release CSB (ends stream mode)

    // Configure PLL feedback divider
    reg_spimaster_config = 0xb002;	// Apply stream mode
    reg_spimaster_data = 0x80;		// Write 0x80 (write mode)
    reg_spimaster_data = 0x12;		// Write 0x12 (start address) PLL feedback divider
    reg_spimaster_data = 0x13;		// Write 0x13 (19 decimal) to PLL feedback divider
    reg_spimaster_config = 0xa102;	// Release CSB (ends stream mode)

    // // activate the project by setting the 0th bit of 2nd bank of LA
    reg_la1_oenb = 0;
    reg_la1_iena = 0;
    reg_la1_data = 1 << 4;  // Pong is project 4 in the MPW

    // reset design with 0bit of 1st bank of LA
    reg_la0_oenb = 0;
    reg_la0_iena = 0;
    reg_la0_data = 1;
    reg_la0_data = 0;

    // no need for anything else as this design is free running.

}
