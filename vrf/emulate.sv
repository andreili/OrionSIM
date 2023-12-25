`timescale 1ps/1ps

module emulate
(
    input   wire                        i_clk,
    input   wire                        i_reset_n
);

    orion_pro_top
    #(
        .TURBO_CLK_10                   (1'b1)
    )
    u_orion_core
    (
        .i_clk                          (i_clk),
        .i_reset_n                      (i_reset_n),
        .i_cfg_sw                       (8'b1000_0111)
    );

endmodule
