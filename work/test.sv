module Adder (
  input logic [31:0] a, // Input 1
  input logic [31:0] b, // Input 2
  output logic [31:0] sum, // Sum of inputs
  output logic [31:0] blackbox_out // Sum of inputs
);

  // Declare internal variables
  logic [31:0] result;

  // Add the inputs
  always_comb begin
    if (a >= 2 && (b <= 10 || a ==0) ) begin
      result = a + b;
    end else begin
      result = a - b;
    end
  end

  // Assign the sum to the output
  assign sum = result;

blackbox bb(result, blackbox_out);


endmodule

