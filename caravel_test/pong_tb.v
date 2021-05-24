// SPDX-FileCopyrightText: 2020 Efabless Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// SPDX-License-Identifier: Apache-2.0

`default_nettype none

`timescale 1 ns / 1 ps

`define SCREENTIMERWIDTH 5
`include "uprj_netlists.v"
`include "caravel_netlists.v"
`include "spiflash.v"


module pong_tb;
    initial begin
        $dumpfile ("pong.vcd");
        $dumpvars (0, pong_tb);
        #1;
    end

    reg clock;
    reg RSTB;
    reg power1, power2;
    reg power3, power4;

    wire gpio;
    wire [37:0] mprj_io;

    // convenience signals
    wire start = mprj_io[8];

    wire player1_a = mprj_io[9];
    wire player1_b = mprj_io[10];
    wire player2_a = mprj_io[11];
    wire player2_b = mprj_io[12];

    wire seg_a = mprj_io[13];
    wire seg_b = mprj_io[14];
    wire seg_c = mprj_io[15];
    wire seg_d = mprj_io[16];
    wire seg_e = mprj_io[17];
    wire seg_f = mprj_io[18];
    wire seg_g = mprj_io[19];
    wire cath  = mprj_io[20];

    wire RCLK  = mprj_io[21];
    wire RSDI  = mprj_io[22];
    wire OEB   = mprj_io[23];
    wire CSDI  = mprj_io[24];
    wire CCLK  = mprj_io[25];
    wire LE    = mprj_io[26];

    wire flash_csb;
    wire flash_clk;
    wire flash_io0;
    wire flash_io1;

    wire VDD3V3 = power1;
    wire VDD1V8 = power2;
    wire USER_VDD3V3 = power3;
    wire USER_VDD1V8 = power4;
    wire VSS = 1'b0;

    caravel uut (
        .vddio	  (VDD3V3),
        .vssio	  (VSS),
        .vdda	  (VDD3V3),
        .vssa	  (VSS),
        .vccd	  (VDD1V8),
        .vssd	  (VSS),
        .vdda1    (USER_VDD3V3),
        .vdda2    (USER_VDD3V3),
        .vssa1	  (VSS),
        .vssa2	  (VSS),
        .vccd1	  (USER_VDD1V8),
        .vccd2	  (USER_VDD1V8),
        .vssd1	  (VSS),
        .vssd2	  (VSS),
        .clock	  (clock),
        .gpio     (gpio),
        .mprj_io  (mprj_io),
        .flash_csb(flash_csb),
        .flash_clk(flash_clk),
        .flash_io0(flash_io0),
        .flash_io1(flash_io1),
        .resetb	  (RSTB)
    );

    spiflash #(
        .FILENAME("pong.hex")
    ) spiflash (
        .csb(flash_csb),
        .clk(flash_clk),
        .io0(flash_io0),
        .io1(flash_io1),
        .io2(),			// not used
        .io3()			// not used
    );

endmodule
`default_nettype wire
