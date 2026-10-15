//-----------------------------------------------------------------------------
// MegaSoC AHB-Lite to APB bridge across unrelated clocks (LPDDR4 config)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// AHB-Lite subordinate on HCLK -> two APB managers on PCLK (the LPDDR4
// subsystem clock). Decodes the 16 MB LPDDR4 configuration window by its low
// address bits (the window itself is selected upstream by HSEL):
//
//   HADDR[23:16] == 8'h00  -> CTRL_APB (APB3), PADDR = HADDR[15:0]   (64 KB)
//   HADDR[23:22] == 2'b01  -> PHY_APB  (APB4), PADDR = HADDR[21:0]   (4 MB)
//   anything else          -> AHB ERROR, no APB access
//
// One transfer at a time, crossed with a request/acknowledge toggle pair:
//   1. AHB side accepts a transfer, captures address/direction/write data,
//      holds HREADYOUT low and flips req_tgl.
//   2. req_tgl crosses to PCLK through a 2-flop synchroniser; on a change the
//      APB side runs SETUP/ACCESS on the selected port until PREADY, captures
//      PRDATA/PSLVERR and flips ack_tgl.
//   3. ack_tgl crosses back; the AHB side completes the data phase with OKAY
//      and the read data, or with a two-cycle ERROR if PSLVERR was set.
// The multi-bit request (address, data, direction, target) and response
// (read data, error) registers are not synchronised: each is held stable from
// before its toggle flips until after the other side has acted on it.
//
// Every access is a full 32-bit word (PSTRB = 4'hF; PHY PPROT = 3'b001); HSIZE is
// ignored. Both resets must come from the same source.
//-----------------------------------------------------------------------------

module megasoc_ahb_to_apb_async (
    // AHB-Lite side
    input  wire             HCLK,
    input  wire             HRESETn,
    ahb.subordinate         AHB,

    // APB side
    input  wire             PCLK,
    input  wire             PRESETn,
    apb3.master             CTRL_APB,
    apb4.master             PHY_APB
);

localparam TGT_CTRL = 2'd0, TGT_PHY = 2'd1, TGT_NONE = 2'd2;

// ---- Request (HCLK domain, read by PCLK domain after req_tgl changes) ----
reg  [21:0] req_addr_q;
reg  [31:0] req_wdata_q;
reg         req_write_q;
reg  [1:0]  req_tgt_q;
reg         req_tgl_q;

// ---- Response (PCLK domain, read by HCLK domain after ack_tgl changes) ----
reg  [31:0] rsp_rdata_q;
reg         rsp_err_q;
reg         ack_tgl_q;

//=============================================================================
// AHB side (HCLK)
//=============================================================================
localparam H_IDLE = 3'd0,   // ready for an address phase
           H_DPH  = 3'd1,   // first data-phase cycle: capture HWDATA, launch
           H_WAIT = 3'd2,   // waiting for the APB side
           H_RESP = 3'd3,   // OKAY: data phase completes this cycle
           H_ERR1 = 3'd4,   // ERROR, first cycle (HREADYOUT low)
           H_ERR2 = 3'd5;   // ERROR, second cycle (HREADYOUT high)

reg  [2:0]  h_state;
reg  [23:0] h_addr_q;
reg         h_write_q;
reg  [31:0] h_rdata_q;
reg         h_ack_sync_q1, h_ack_sync_q2, h_ack_seen_q;

wire        h_accept   = AHB.HSEL & AHB.HTRANS[1] & AHB.HREADY;
wire        h_ack_new  = (h_ack_sync_q2 != h_ack_seen_q);
wire        h_can_take = (h_state == H_IDLE) | (h_state == H_RESP) | (h_state == H_ERR2);

wire [1:0]  h_tgt = (h_addr_q[23:16] == 8'h00) ? TGT_CTRL :
                    (h_addr_q[23:22] == 2'b01) ? TGT_PHY  : TGT_NONE;

assign AHB.HREADYOUT = (h_state == H_IDLE) | (h_state == H_RESP) | (h_state == H_ERR2);
assign AHB.HRESP     = (h_state == H_ERR1) | (h_state == H_ERR2);
assign AHB.HRDATA    = h_rdata_q;

always @ (posedge HCLK or negedge HRESETn)
  begin
    if (!HRESETn)
      begin
        h_state       <= H_IDLE;
        h_addr_q      <= 24'd0;
        h_write_q     <= 1'b0;
        h_rdata_q     <= 32'd0;
        h_ack_sync_q1 <= 1'b0;
        h_ack_sync_q2 <= 1'b0;
        h_ack_seen_q  <= 1'b0;
        req_addr_q    <= 22'd0;
        req_wdata_q   <= 32'd0;
        req_write_q   <= 1'b0;
        req_tgt_q     <= TGT_NONE;
        req_tgl_q     <= 1'b0;
      end
    else
      begin
        h_ack_sync_q1 <= ack_tgl_q;
        h_ack_sync_q2 <= h_ack_sync_q1;

        case (h_state)
          H_DPH:
            if (h_tgt == TGT_NONE)
              h_state <= H_ERR1;
            else
              begin
                req_addr_q  <= h_addr_q[21:0];
                req_wdata_q <= AHB.HWDATA;
                req_write_q <= h_write_q;
                req_tgt_q   <= h_tgt;
                req_tgl_q   <= ~req_tgl_q;
                h_state     <= H_WAIT;
              end

          H_WAIT:
            if (h_ack_new)
              begin
                h_ack_seen_q <= h_ack_sync_q2;
                h_rdata_q    <= rsp_rdata_q;
                h_state      <= rsp_err_q ? H_ERR1 : H_RESP;
              end

          H_ERR1:
            h_state <= H_ERR2;

          default: // H_IDLE, H_RESP, H_ERR2: data phase (if any) completes now
            h_state <= H_IDLE;
        endcase

        // New address phase, taken when the previous data phase completes
        if (h_can_take && h_accept)
          begin
            h_addr_q  <= AHB.HADDR[23:0];
            h_write_q <= AHB.HWRITE;
            h_state   <= H_DPH;
          end
      end
  end

//=============================================================================
// APB side (PCLK)
//=============================================================================
localparam P_IDLE = 2'd0, P_SETUP = 2'd1, P_ACCESS = 2'd2;

reg  [1:0]  p_state;
reg         p_req_sync_q1, p_req_sync_q2, p_req_seen_q;
reg  [21:0] p_addr_q;
reg  [31:0] p_wdata_q;
reg         p_write_q;
reg         p_phy_q;

wire        p_req_new = (p_req_sync_q2 != p_req_seen_q);
wire        p_active  = (p_state != P_IDLE);
wire        p_enable  = (p_state == P_ACCESS);
wire        p_ready   = p_phy_q ? PHY_APB.pready  : CTRL_APB.pready;
wire        p_slverr  = p_phy_q ? PHY_APB.pslverr : CTRL_APB.pslverr;
wire [31:0] p_rdata   = p_phy_q ? PHY_APB.prdata  : CTRL_APB.prdata;

assign CTRL_APB.psel    = p_active & ~p_phy_q;
assign CTRL_APB.penable = p_enable & ~p_phy_q;
assign CTRL_APB.paddr   = {16'd0, p_addr_q[15:0]};
assign CTRL_APB.pwrite  = p_write_q;
assign CTRL_APB.pwdata  = p_wdata_q;

assign PHY_APB.psel     = p_active & p_phy_q;
assign PHY_APB.penable  = p_enable & p_phy_q;
assign PHY_APB.paddr    = {10'd0, p_addr_q};
assign PHY_APB.pwrite   = p_write_q;
assign PHY_APB.pwdata   = p_wdata_q;
// The LPDDR4 subsystem configures its PHY to accept only APB accesses with
// protection 3'b001 and to ignore accesses that do not match, so
// present every configuration access as privileged (same as the subsystem's
// standalone testbench). Access control for this window belongs at SoC level.
assign PHY_APB.pprot    = 3'b001;
assign PHY_APB.pstrb    = 4'hF;

always @ (posedge PCLK or negedge PRESETn)
  begin
    if (!PRESETn)
      begin
        p_state       <= P_IDLE;
        p_req_sync_q1 <= 1'b0;
        p_req_sync_q2 <= 1'b0;
        p_req_seen_q  <= 1'b0;
        p_addr_q      <= 22'd0;
        p_wdata_q     <= 32'd0;
        p_write_q     <= 1'b0;
        p_phy_q       <= 1'b0;
        rsp_rdata_q   <= 32'd0;
        rsp_err_q     <= 1'b0;
        ack_tgl_q     <= 1'b0;
      end
    else
      begin
        p_req_sync_q1 <= req_tgl_q;
        p_req_sync_q2 <= p_req_sync_q1;

        case (p_state)
          P_IDLE:
            if (p_req_new)
              begin
                p_req_seen_q <= p_req_sync_q2;
                p_addr_q     <= req_addr_q;
                p_wdata_q    <= req_wdata_q;
                p_write_q    <= req_write_q;
                p_phy_q      <= (req_tgt_q == TGT_PHY);
                p_state      <= P_SETUP;
              end

          P_SETUP:
            p_state <= P_ACCESS;

          P_ACCESS:
            if (p_ready)
              begin
                rsp_rdata_q <= p_write_q ? 32'd0 : p_rdata;
                rsp_err_q   <= p_slverr;
                ack_tgl_q   <= ~ack_tgl_q;
                p_state     <= P_IDLE;
              end

          default:
            p_state <= P_IDLE;
        endcase
      end
  end

endmodule
