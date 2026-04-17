module megasoc_reset_ctrl #(
    parameter int ADDR_WIDTH = 12
)(
input logic PCLK,
input logic PRESETn,
input logic PSEL,
input logic PENABLE,
input logic PWRITE,
input logic [ADDR_WIDTH - 1: 0] PADDR,
input logic [31:0] PWDATA,
output logic [31:0] PRDATA,
output logic PREADY,
output logic PSLVERR,

// Always on domain
input logic FCLK,    
input logic PORESETn,

//Reset sources
input logic wdog_reset_req_i,
input logic dbg_reset_req_i,
input logic lockup_reset_req_i,

//Reset controller outputs
output logic rst_ahb_ctrl_n,
output logic rst_apb_ctrl_n,
output logic rst_dbg_ctrl_n,
output logic sys_reset_req
);

    assign PREADY = 1'b1; //No wait states always ready
    assign PSLVERR = 1'b0; 

    localparam logic [ADDR_WIDTH - 1: 2] ADDR_RESET_REQ = 'h000;  // Becasue APB registers are word addressible
    localparam logic [ADDR_WIDTH - 1: 2] ADDR_RESET_STATUS = 'h001;

// RESET_REQ bits (write only preferred)
    localparam int RESET_REQ_SWRESET_BIT   = 0;
    localparam int RESET_REQ_WDOGRESET_BIT = 1;
    localparam int RESET_REQ_DBGRESET_BIT  = 2;
    localparam int RESET_REQ_LOCKUP_BIT    = 3;    

// RESET_STATUS bits
    localparam int RESET_STS_SWRESET_BIT   = 0;
    localparam int RESET_STS_WDOG_BIT      = 1;
    localparam int RESET_STS_DBG_BIT       = 2;
    localparam int RESET_STS_LOCKUP_BIT    = 3;
    localparam int RESET_STS_PORESET_BIT   = 4;
    
// Write decoding in PCLK Domain either RESET_REQ or RESET_STATUS. When data is written into a either 
// of the registers.

    logic wr_en, rd_en;
    logic addr_is_req, addr_is_status;

    assign wr_en = PRESETn & PSEL & PWRITE & PENABLE;  //PENABLE where access phase starts.
    assign rd_en = PRESETn & PSEL & ~PWRITE;
    assign addr_is_req = (PADDR[ADDR_WIDTH - 1: 2] == ADDR_RESET_REQ);
    assign addr_is_status = (PADDR[ADDR_WIDTH - 1: 2] == ADDR_RESET_STATUS);

// One cycle pulses in PCLK
    logic sw_reset_req_pulse_pclk;
    logic wdog_reset_req_pulse_pclk;
    logic dbg_reset_req_pulse_pclk;
    logic lockup_reset_req_pulse_pclk;
    logic clear_status_pclk;
    logic [4:0] clear_status_mask_pclk;
    
// Write data implementation
    always_ff @ (posedge PCLK or negedge PRESETn)
    begin
        if (!PRESETn)
        begin
            sw_reset_req_pulse_pclk <= 1'b0;
            wdog_reset_req_pulse_pclk <= 1'b0;
            dbg_reset_req_pulse_pclk <= 1'b0;
            lockup_reset_req_pulse_pclk <= 1'b0;
            clear_status_pclk <= 1'b0;
            clear_status_mask_pclk <= 5'b0;
        end else begin
            sw_reset_req_pulse_pclk <= 1'b0;
            wdog_reset_req_pulse_pclk <= 1'b0;
            dbg_reset_req_pulse_pclk <= 1'b0;
            lockup_reset_req_pulse_pclk <= 1'b0;
            clear_status_pclk <= 1'b0;

            if (wr_en && addr_is_req) begin
                sw_reset_req_pulse_pclk <= PWDATA[RESET_REQ_SWRESET_BIT];
                wdog_reset_req_pulse_pclk <= PWDATA[RESET_REQ_WDOGRESET_BIT];
                dbg_reset_req_pulse_pclk <= PWDATA[RESET_REQ_DBGRESET_BIT];
                lockup_reset_req_pulse_pclk <= PWDATA[RESET_REQ_LOCKUP_BIT];
            end

            if (wr_en && addr_is_status) begin
                clear_status_pclk <= 1'b1;  // Write 1 to clear reset clear_status_pclk
                clear_status_mask_pclk <= PWDATA[4:0];
            end
        end
    end

// APB Read data
    logic [31:0] reset_status_ro;
    always_comb begin
        PRDATA = 32'h0;
        if (rd_en && addr_is_status) begin
            PRDATA[4:0] = reset_status_ro[4:0];
        end
    end

// As reset requests are pulses to avoid any CDC issues signals are passed through 2 stage synchronous FF.

// Sync SW reset request pulse
    logic sw_req_sync1, sw_req_sync2;
    logic dbg_req_sync1, dbg_req_sync2;
    logic lockup_req_sync1, lockup_req_sync2;

    always_ff @(posedge FCLK or negedge PORESETn)
    begin
    if (!PORESETn) begin
        sw_req_sync1 <= 1'b0;
        sw_req_sync2 <= 1'b0;
        dbg_req_sync1 <= 1'b0;
        dbg_req_sync2 <= 1'b0;
        lockup_req_sync1 <= 1'b0;
        lockup_req_sync2 <= 1'b0;
    end else begin
        sw_req_sync1 <= sw_reset_req_pulse_pclk;
        sw_req_sync2 <= sw_req_sync1;

        dbg_req_sync1 <= dbg_reset_req_pulse_pclk;
        dbg_req_sync2 <= dbg_req_sync1;

        lockup_req_sync1 <= lockup_reset_req_pulse_pclk;
        lockup_req_sync2 <= lockup_req_sync1;
        end
    end

// Edge Clock detection in FCLK domain for 1 clock pulse when data changes from 0 to 1

    logic sw_reset_req_pulse_fclk;
    logic dbg_reset_req_pulse_fclk;
    logic lockup_reset_req_pulse_fclk;
    
    always_comb begin
        sw_reset_req_pulse_fclk = sw_req_sync1 & ~sw_req_sync2;
        dbg_reset_req_pulse_fclk = dbg_req_sync1 & ~dbg_req_sync2;
        lockup_reset_req_pulse_fclk = lockup_req_sync1 & ~lockup_req_sync2;
    end

// RESET_STATUS in FCLK Domain

    logic [4:0] reset_status_reg;
    logic clear_status_fclk;
    logic [4:0] clear_status_mask_fclk;

    logic clr_sync1, clr_sync2;
    logic [4:0] clr_mask_sync1, clr_mask_sync2;

    always_ff @(posedge FCLK or negedge PORESETn)
    begin
        if (!PORESETn) begin
            clr_sync1 <= 0;
            clr_sync2 <= 0;
            clr_mask_sync1 <= 5'b0;
            clr_mask_sync2 <= 5'b0;
        end else 
        begin
            clr_sync1 <= clear_status_pclk;
            clr_sync2 <= clr_sync1;
            clr_mask_sync1 <= clear_status_mask_pclk;
            clr_mask_sync2 <= clr_mask_sync1;
        end
    end
    // No edge detection stores the value for debugging purpose at later stage
    assign clear_status_fclk = clr_sync2;
    assign clear_status_mask_fclk = clr_mask_sync2;

    always_ff @(posedge FCLK or negedge PORESETn) 
    begin
        if (!PORESETn) begin
            reset_status_reg <= 5'b0;
            reset_status_reg [RESET_STS_PORESET_BIT] <= 1'b1;
        end else begin
            if (sw_reset_req_pulse_fclk) reset_status_reg[RESET_STS_SWRESET_BIT] <= 1'b1;
            if (wdog_reset_req_i) reset_status_reg[RESET_STS_WDOG_BIT] <= 1'b1;  //wdog_reset_req_i is a level, not a pulse need to check.
            if (lockup_reset_req_i) reset_status_reg[RESET_STS_LOCKUP_BIT] <=1'b1;
            if (dbg_reset_req_i) reset_status_reg[RESET_STS_DBG_BIT] <= 1'b1;
            if (clear_status_fclk) reset_status_reg <= reset_status_reg & ~clear_status_mask_fclk;
            /////////////
        end
    end

    assign reset_status_ro = {27'b0, reset_status_reg};



    logic porestn_d;

    always_ff @(posedge FCLK or negedge PORESETn) begin
        if (!PORESETn)
            porestn_d <= 1'b0;
        else
            porestn_d <= 1'b1;
    end

    logic por_release_pulse;
    assign por_release_pulse = PORESETn & ~porestn_d; // one FCLK cycle after POR release


    logic global_reset_req;
    assign global_reset_req = por_release_pulse | sw_reset_req_pulse_fclk | wdog_reset_req_i | lockup_reset_req_i | dbg_reset_req_i;

    logic reset_latch; //latch keeps active till the FSM routine completes
    typedef enum logic [2:0] {IDLE, ASSERT_ALL, R_AHB, R_APB, DONE} state_t;
    state_t state, next_state;
    logic [2:0] delay_count;

    always_ff @(posedge FCLK or negedge PORESETn) begin
        if (!PORESETn) begin
            reset_latch <= 1'b0;
            state <= ASSERT_ALL;
            delay_count <= 3'd0;
        end else begin
            if (global_reset_req) reset_latch <= 1'b1;
            else if (state == DONE) reset_latch <= 1'b0;

            state <= next_state;

            // Delay counter - reset on state change, else increament
            if (next_state != state) delay_count <= 3'd0;
            else if (state != IDLE && state != DONE) delay_count <= delay_count + 1;

        end
    end

    assign sys_reset_req = reset_latch;

    always_comb begin
        next_state = state;

        unique case (state)
            IDLE: if (reset_latch) next_state = ASSERT_ALL;
            ASSERT_ALL: if (delay_count == 3'd3) next_state = R_AHB;
            R_AHB: if (delay_count == 3'd3) next_state = R_APB;
            R_APB: if (delay_count == 3'd3) next_state = DONE;
            DONE: if (!reset_latch) next_state = IDLE;
            default: next_state = ASSERT_ALL;
        endcase
    end

    always_comb begin
        rst_ahb_ctrl_n = 1'b0;
        rst_apb_ctrl_n = 1'b0;
        rst_dbg_ctrl_n = 1'b1;  //Need to verify its active high/low
        //Considering Not in reset by default 

        unique case (state)
            ASSERT_ALL: begin
                rst_ahb_ctrl_n = 1'b0;
                rst_apb_ctrl_n = 1'b0;

                if (reset_latch) rst_dbg_ctrl_n = 1'b0;
                else rst_dbg_ctrl_n = 1'b1;
            end

            R_AHB: begin
                rst_ahb_ctrl_n = 1'b1;
                rst_apb_ctrl_n = 1'b0;
                rst_dbg_ctrl_n = 1'b1;
            end

            R_APB: begin
                rst_ahb_ctrl_n = 1'b1;
                rst_apb_ctrl_n = 1'b1;
                rst_dbg_ctrl_n = 1'b1;
            end

            DONE: begin
                rst_ahb_ctrl_n = 1'b1;
                rst_apb_ctrl_n = 1'b1;
                rst_dbg_ctrl_n = 1'b1;
            end

            IDLE: begin
                rst_ahb_ctrl_n = 1'b1;
                rst_apb_ctrl_n = 1'b1;
                rst_dbg_ctrl_n = 1'b1;
            end

            default: begin
                rst_ahb_ctrl_n = 1'b0;
                rst_apb_ctrl_n = 1'b0;
                rst_dbg_ctrl_n = 1'b1;
            end
        endcase
    end

endmodule