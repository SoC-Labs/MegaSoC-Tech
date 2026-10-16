//-----------------------------------------------------------------------------
// MegaSoC Q-Channel to AMBA low-power interface adapter (LPDDR4 subsystem)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// The LPDDR4 subsystem exposes the controller's two AMBA low-power interfaces
// (AXI port, controller core) as Q-channels by plain renaming:
//   csysreq <- QREQn,  QACCEPTn <- csysack,  QACTIVE <- cactive,  QDENY = 0
// That mapping is wrong for a refused request: on the low-power interface a
// peripheral denies by driving CSYSACK LOW while CACTIVE stays HIGH, which the
// plain mapping shows as an acceptance. This adapter sits between a Q-channel
// controller (the DRAM power domain's request distributor, PCLK domain) and
// one subsystem channel (DEV, DDR_CLK domain) and implements the handshake
// properly:
//
//   Q-channel (CTRL)          adapter           low-power interface (DEV)
//   RUN      QREQn=1  ------------------------> csysreq=1
//   REQUEST  QREQn=0  ------------------------> csysreq=0, wait csysack=0
//            accept (cactive=0 at ack): QACCEPTn=0            -> STOPPED
//            deny   (cactive=1 at ack): QDENY=1, csysreq=1     -> DENIED
//   STOPPED  QREQn=1  ------------------------> csysreq=1, wait csysack=1
//            then QACCEPTn=1                                    -> RUN
//   DENIED   wait QREQn=1 and csysack=1, then QDENY=0           -> RUN
//
// Clocking: everything runs on clk (the DEV side clock). CTRL.QREQn is
// synchronised with two flops; QACCEPTn/QDENY are driven from flops so the
// controller can synchronise them. QACTIVE is passed through from cactive
// (asynchronous to the controller; synchronise it there).
//
// Reset: if CTRL.QREQn is LOW when reset ends, the adapter starts STOPPED
// (Q-channel Q_STOPPED) without touching the low-power interface -- the DRAM
// controller is still held in reset by firmware at that point -- and leaves
// it on the first QREQn rise without a low-power-interface exit handshake.
//-----------------------------------------------------------------------------

module megasoc_lpi_qchannel_adapter (
    input  wire         clk,
    input  wire         resetn,

    qchannel.subordinate CTRL,   // from the Q-channel controller
    qchannel.master      DEV     // to the subsystem channel (csysreq/csysack/cactive)
);

localparam S_RUN       = 3'd0,
           S_REQUEST   = 3'd1,
           S_STOPPED   = 3'd2,
           S_EXIT      = 3'd3,
           S_DENIED    = 3'd4,
           S_RST_STOP  = 3'd5;   // stopped out of reset, interface never asked

reg  [2:0] state;
reg        qreqn_s1, qreqn_s2;   // CTRL.QREQn synchroniser
reg  [1:0] fill;                 // clocks since reset (synchroniser filling)
reg        started;              // reset level of QREQn has been taken
reg        csysreq_q;
reg        qacceptn_q;
reg        qdeny_q;

wire       csysack = DEV.qacceptn;
wire       cactive = DEV.qactive;

assign DEV.qreqn     = csysreq_q;
assign CTRL.qacceptn = qacceptn_q;
assign CTRL.qdeny    = qdeny_q;
assign CTRL.qactive  = cactive;

always @(posedge clk or negedge resetn)
  begin
    if (!resetn)
      begin
        qreqn_s1       <= 1'b1;
        qreqn_s2       <= 1'b1;
        fill           <= 2'd0;
        started        <= 1'b0;
        state          <= S_RUN;
        csysreq_q      <= 1'b1;
        qacceptn_q     <= 1'b1;
        qdeny_q        <= 1'b0;
      end
    else
      begin
        qreqn_s1 <= CTRL.qreqn;
        qreqn_s2 <= qreqn_s1;

        if (!started)
          begin
            // Wait for the synchroniser to fill, then follow the controller's
            // reset level: QREQn LOW means Q_STOPPED. (Until then a LOW QREQn
            // looks like a request not yet answered, which is legal.)
            if (fill == 2'd2)
              begin
                started <= 1'b1;
                if (!qreqn_s2)
                  begin
                    state      <= S_RST_STOP;
                    qacceptn_q <= 1'b0;
                  end
              end
            else
              fill <= fill + 2'd1;
          end
        else
          case (state)
            S_RUN:
              if (!qreqn_s2)
                begin
                  csysreq_q <= 1'b0;
                  state     <= S_REQUEST;
                end

            S_REQUEST:
              if (!csysack)
                begin
                  if (cactive)
                    begin                    // refused: finish the interface handshake
                      qdeny_q   <= 1'b1;
                      csysreq_q <= 1'b1;
                      state     <= S_DENIED;
                    end
                  else
                    begin
                      qacceptn_q <= 1'b0;
                      state      <= S_STOPPED;
                    end
                end

            S_STOPPED:
              if (qreqn_s2)
                begin
                  csysreq_q <= 1'b1;
                  state     <= S_EXIT;
                end

            S_EXIT:
              if (csysack)
                begin
                  qacceptn_q <= 1'b1;
                  state      <= S_RUN;
                end

            S_DENIED:
              if (qreqn_s2 && csysack)
                begin
                  qdeny_q <= 1'b0;
                  state   <= S_RUN;
                end

            S_RST_STOP:
              if (qreqn_s2)
                begin
                  qacceptn_q <= 1'b1;
                  state      <= S_RUN;
                end

            default:
              state <= S_RUN;
          endcase
      end
  end

endmodule
