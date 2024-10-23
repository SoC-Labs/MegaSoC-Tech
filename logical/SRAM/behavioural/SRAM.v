

module SRAM #(
  parameter MEM_DEPTH = (1<<15)
) (
    input  wire           clk,
    input  wire [19:0]    memaddr,
    input  wire [63:0]    memd,
    output wire [63:0]    memq,
    input  wire           memcen,
    input  wire [7:0]     memwen
);

  wire                WriteEnable;        // Write data update
  wire   [15:0]       Addr;
  reg    [63:0]      DataAtAddress;      // Current write-data at address
  reg    [63:0]      Mask;               // Write data-mask
  reg    [63:0]      NextData;           // Next write-data
  reg    [63:0]      iQ;   // Memory output data (pipelined)

  integer                  i;     // Write-strobe loop variable
  integer                  j;     // Mask-bit loop variable
  assign Addr = memaddr[19:3];
  // -------------
  // Memory arrays
  // -------------

  // Memory array 0 - used in both 32-bit and 64-bit modes
  reg    [63:0]           mem [MEM_DEPTH-1:0];
  assign WriteEnable = (memwen != {8{1'b1}}) ? 1'b1 : 1'b0;

  integer k;
  initial 
    begin 
      for(k=0;k<MEM_DEPTH;k=k+1) begin 
        mem[k] = 64'd0;
      end 
    end
    

  always @ (posedge clk)
    begin : p_memaccess
      // Only access the memory when the chip is enabled
      if (!memcen)
        begin
          // Look-up the data at the current address
          DataAtAddress[63:0] = mem[Addr]; 

          // Update the memory and the data output only when permitted
          if (WriteEnable)
            begin

              // Determine the byte-lane mask value by testing the individual
              //  bits of the active-low write strobes
              for (i = 0; i < 8; i = i + 1)
                for (j = i * 8; j <= (i * 8) + 7; j = j + 1)
                  Mask[j] = ~memwen[i];

              // Determine the value of the next write-data. Term (a) clears
              //  the required byte lanes and term (b) selects the required
              //  byte-lanes of the AXI write data. The two data words are
              //  bit-wise OR'ed together to form the new data word
              NextData = (DataAtAddress & ~Mask) |             // (a)
                         (memd & Mask);                           // (b)

              mem[Addr] = NextData[63:0];    // Always assign mem array 0

              // Update the data output with new data
              iQ <= NextData;

            end
          else
            // Update the data output with the original data value
            iQ <= DataAtAddress;

        end


    end


  // Drive read data output port at the selected stage of the pipeline
  assign memq = iQ;


endmodule
