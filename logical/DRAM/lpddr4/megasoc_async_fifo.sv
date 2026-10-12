//-----------------------------------------------------------------------------
// MegaSoC asynchronous FIFO with valid/ready handshakes on both sides
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Moves WIDTH-bit words from the write clock domain to an unrelated read
// clock domain. Each side behaves like one AXI channel:
//   write side: push on (w_valid && w_ready), w_ready = not full
//   read side : pop  on (r_valid && r_ready), r_valid = not empty,
//               r_data is the oldest word (first-word fall-through)
//
// Classic gray-code pointer scheme:
//   - each side keeps its pointer in binary (to address the storage) and in
//     gray code (only one bit changes per increment, so a pointer sampled
//     mid-change in the other domain is either the old or the new value,
//     never a mix);
//   - the gray pointer is passed to the other domain through a 2-flop
//     synchroniser (the *_sync_q* registers -- CDC tools should treat them
//     as synchronisers);
//   - pointers are one bit wider than the address so full (same address,
//     different wrap bit) and empty (identical) can be told apart.
// full/empty are pessimistic (they see the other side's pointer 2-3 cycles
// late) which is always safe: the FIFO may briefly look fuller or emptier
// than it is, never the reverse.
//
// Storage is written in the write domain and read combinationally in the
// read domain. A word is only visible to the read side after its write
// pointer has crossed the synchroniser, by which time it is stable.
//
// Reset: both resets must be asserted together (one reset source,
// de-asserted synchronously in each domain); resetting only one side while
// the other has words in flight loses or duplicates data.
//-----------------------------------------------------------------------------

module megasoc_async_fifo #(
    parameter WIDTH = 8,
    parameter DEPTH = 4       // power of two, >= 2
    )(
    // Write (producer) domain
    input  wire             w_clk,
    input  wire             w_resetn,
    input  wire             w_valid,
    output wire             w_ready,
    input  wire [WIDTH-1:0] w_data,

    // Read (consumer) domain
    input  wire             r_clk,
    input  wire             r_resetn,
    output wire             r_valid,
    input  wire             r_ready,
    output wire [WIDTH-1:0] r_data
);

localparam AW = $clog2(DEPTH);   // storage address bits
localparam PW = AW + 1;          // pointer bits (extra wrap bit)
localparam [PW-1:0] FULL_MASK = 2'b11 << (PW-2);   // top two pointer bits

// synthesis translate_off
initial
  begin
    if ((DEPTH < 2) || (DEPTH != (1 << AW)))
      $fatal(1, "megasoc_async_fifo: DEPTH (%0d) must be a power of two >= 2", DEPTH);
  end
// synthesis translate_on

reg [WIDTH-1:0] mem [0:DEPTH-1];

reg  [PW-1:0] w_bin_q, w_gray_q;                  // write pointer (binary, gray)
reg  [PW-1:0] r_bin_q, r_gray_q;                  // read pointer (binary, gray)

function automatic [PW-1:0] bin2gray(input [PW-1:0] bin);
  bin2gray = bin ^ (bin >> 1);
endfunction

// ---- Write domain ----
reg  [PW-1:0] w_rgray_sync_q1, w_rgray_sync_q2;   // read pointer, synchronised into w_clk
reg           w_full_q;

wire          w_push     = w_valid & ~w_full_q;
wire [PW-1:0] w_bin_nxt  = w_bin_q + {{(PW-1){1'b0}}, w_push};
wire [PW-1:0] w_gray_nxt = bin2gray(w_bin_nxt);

// Full: write pointer one lap ahead of the read pointer. In gray code that
// is "top two bits inverted, the rest equal".
wire          w_full_nxt = (w_gray_nxt == (w_rgray_sync_q2 ^ FULL_MASK));

assign w_ready = ~w_full_q;

always @ (posedge w_clk)
  begin
    if (w_push)
      mem[w_bin_q[AW-1:0]] <= w_data;
  end

always @ (posedge w_clk or negedge w_resetn)
  begin
    if (!w_resetn)
      begin
        w_bin_q         <= {PW{1'b0}};
        w_gray_q        <= {PW{1'b0}};
        w_full_q        <= 1'b0;
        w_rgray_sync_q1 <= {PW{1'b0}};
        w_rgray_sync_q2 <= {PW{1'b0}};
      end
    else
      begin
        w_bin_q         <= w_bin_nxt;
        w_gray_q        <= w_gray_nxt;
        w_full_q        <= w_full_nxt;
        w_rgray_sync_q1 <= r_gray_q;
        w_rgray_sync_q2 <= w_rgray_sync_q1;
      end
  end

// ---- Read domain ----
reg  [PW-1:0] r_wgray_sync_q1, r_wgray_sync_q2;   // write pointer, synchronised into r_clk
reg           r_empty_q;

wire          r_pop      = r_ready & ~r_empty_q;
wire [PW-1:0] r_bin_nxt  = r_bin_q + {{(PW-1){1'b0}}, r_pop};
wire [PW-1:0] r_gray_nxt = bin2gray(r_bin_nxt);
wire          r_empty_nxt = (r_gray_nxt == r_wgray_sync_q2);

assign r_valid = ~r_empty_q;
assign r_data  = mem[r_bin_q[AW-1:0]];

always @ (posedge r_clk or negedge r_resetn)
  begin
    if (!r_resetn)
      begin
        r_bin_q         <= {PW{1'b0}};
        r_gray_q        <= {PW{1'b0}};
        r_empty_q       <= 1'b1;
        r_wgray_sync_q1 <= {PW{1'b0}};
        r_wgray_sync_q2 <= {PW{1'b0}};
      end
    else
      begin
        r_bin_q         <= r_bin_nxt;
        r_gray_q        <= r_gray_nxt;
        r_empty_q       <= r_empty_nxt;
        r_wgray_sync_q1 <= w_gray_q;
        r_wgray_sync_q2 <= r_wgray_sync_q1;
      end
  end

endmodule
