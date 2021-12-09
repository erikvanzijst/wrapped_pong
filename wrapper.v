`default_nettype none
`ifdef FORMAL
    `define MPRJ_IO_PADS 38
`endif

`ifndef SCREENTIMERWIDTH
    // Used to override the matrix screen refresh time during tests
    `define SCREENTIMERWIDTH 10
`endif

// update this to the name of your module
module wrapped_pong(
`ifdef USE_POWER_PINS
    inout vccd1,	// User area 1 1.8V supply
    inout vssd1,	// User area 1 digital ground
`endif
    // wishbone interface
    input wire wb_clk_i,            // system clock -- ~31.5MHz from soc PLL set in firmware
    input wire wb_rst_i,            // main system reset
    input wire wbs_stb_i,           // wishbone write strobe
    input wire wbs_cyc_i,           // wishbone cycle
    input wire wbs_we_i,            // wishbone write enable
    input wire [3:0] wbs_sel_i,     // wishbone write word select
    input wire [31:0] wbs_dat_i,    // wishbone data in
    input wire [31:0] wbs_adr_i,    // wishbone address
    output wire wbs_ack_o,          // wishbone ack
    output wire [31:0] wbs_dat_o,   // wishbone data out

    // Logic Analyzer Signals
    // only provide first 32 bits to reduce wiring congestion
    input  wire [31:0] la_data_in,  // from PicoRV32 to your project
    output wire [31:0] la_data_out, // from your project to PicoRV32
    input  wire [31:0] la_oenb,     // output enable bar (low for active)

    // IOs
    input  wire [`MPRJ_IO_PADS-1:0] io_in,  // in to your project
    output wire [`MPRJ_IO_PADS-1:0] io_out, // out fro your project
    output wire [`MPRJ_IO_PADS-1:0] io_oeb, // out enable bar (low active)

    // IRQ
    output wire [2:0] irq,          // interrupt from project to PicoRV32

    // extra user clock
    input wire user_clock2,
    
    // active input, only connect tristated outputs if this is high
    input wire active
);
    // Tiny clock 3-to-1 devider to generate a ~10.5MHz clock for the game
    // logic.
    // We can't get to an accurate 12MHz clock this way, so it would be better
    // if the PLL's second divider output (register address 0x11, bit 5–3)
    // could be passed on to the user space area so as to offer a second
    // configurable clock.

    // all outputs must be tristated before being passed onto the project
    wire buf_wbs_ack_o;
    wire [31:0] buf_wbs_dat_o;
    wire [31:0] buf_la_data_out;
    wire [`MPRJ_IO_PADS-1:0] buf_io_out;
    wire [`MPRJ_IO_PADS-1:0] buf_io_oeb;
    wire [2:0] buf_irq;

    `ifdef FORMAL
    // formal can't deal with z, so set all outputs to 0 if not active
    assign wbs_ack_o    = active ? buf_wbs_ack_o    : 1'b0;
    assign wbs_dat_o    = active ? buf_wbs_dat_o    : 32'b0;
    assign la_data_out  = active ? buf_la_data_out  : 32'b0;
    assign io_out       = active ? buf_io_out       : {`MPRJ_IO_PADS{1'b0}};
    assign io_oeb       = active ? buf_io_oeb       : {`MPRJ_IO_PADS{1'b0}};
    assign irq          = active ? buf_irq          : 3'b0;
    `include "properties.v"
    `else
    // tristate buffers
    assign wbs_ack_o    = active ? buf_wbs_ack_o    : 1'bz;
    assign wbs_dat_o    = active ? buf_wbs_dat_o    : 32'bz;
    assign la_data_out  = active ? buf_la_data_out  : 32'bz;
    assign io_out       = active ? buf_io_out       : {`MPRJ_IO_PADS{1'bz}};
    assign io_oeb       = active ? buf_io_oeb       : {`MPRJ_IO_PADS{1'bz}};
    assign irq          = active ? buf_irq          : 3'bz;
    `endif

    // permanently set oeb so that outputs are always enabled: 0 is output, 1 is high-impedance
    assign buf_io_oeb = {`MPRJ_IO_PADS{1'b0}};

    // Instantiate your module here, 
    // connecting what you need of the above signals. 
    // Use the buffered outputs for your module's outputs.

    pong #(
    `ifdef GAMECLK
        // Used to override the game speed during tests
        .GAMECLK(`GAMECLK),
    `endif
    `ifdef DEBOUNCEWIDTH
        // Can be override during tests
        .DEBOUNCEWIDTH(`DEBOUNCEWIDTH),
    `endif
    .SCREENTIMERWIDTH(`SCREENTIMERWIDTH)) pong0 (
        .clk32mhz(wb_clk_i),
        .reset(la_data_in[0]),
        
        .start(io_in[8]),

        // Difficulty:
        .difficulty(io_in[36:33]),

        .player1_a(io_in[9]),
        .player1_b(io_in[10]),
        .player2_a(io_in[11]),
        .player2_b(io_in[12]),

        // BCD scoreboards:
        .score(buf_io_out[16:13]),
        .cath1(buf_io_out[17]),
        .cath2(buf_io_out[18]),

        // 16x16 Matrix display:
        .RCLK(buf_io_out[19]),
        .RSDI(buf_io_out[20]),
        .OEB(buf_io_out[21]),
        .CSDI(buf_io_out[22]),
        .CCLK(buf_io_out[23]),
        .LE(buf_io_out[24]),

        // VGA port:
        .hsync(buf_io_out[25]),
        .vsync(buf_io_out[26]),
        .rrggbb(buf_io_out[32:27]),

        // Passive piezo buzzer:
        .buzzer(buf_io_out[37]),

        // Debugging:
        .x(buf_la_data_out[4:0]),
        .y(buf_la_data_out[9:5])
    );

endmodule 
`default_nettype wire
