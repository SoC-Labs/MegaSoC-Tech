//-----------------------------------------------------------------------------
// MegaSoC Interrupt synchronizer
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Purpose:
//  Synchronize interrupts to input clock. Needed as GIC has no internal synchronizers
//-----------------------------------------------------------------------------
// Modules instantiated:
//  none


module megasoc_irq_sync #(
    parameter NUM_SPIS=240
    )(
        input wire  CLK,
        input wire  RESETn,
        input wire [NUM_SPIS-1:0]   SPI_i,
        output reg [NUM_SPIS-1:0]   SPI_o
);

always @(posedge CLK or negedge RESETn) begin 
    if(~RESETn) begin 
        SPI_o<={NUM_SPIS{1'b0}};
    end else begin 
        SPI_o<=SPI_i;
    end
end
endmodule