`timescale 1ps/1ps

/* verilator lint_off UNUSEDSIGNAL */
module orion_pro_top
#(
    parameter logic TURBO_CLK_10 = 1'b1
)
(
    input   wire                        i_clk,
    input   wire                        i_reset_n,
    input   wire[7:0]                   i_cfg_sw
);

////
    logic blio = 1'b1;
////

    logic o128_mode, ram0_en, ram1_en, ram2_en, rom1_en, rom2_en, turbo_n, top_mem_en;
    logic[7:0] ram_page_pro, rom2_addr, ram0_page, ram1_page, ram2_page;
    logic ctrl_FB_erom, ctrl_FB_xmem, ctrl_FB_int, ctrl_FB_mz;
    logic[1:0] ctrl_FB_SS;
    logic[2:0] ctrl_FB_BS;

    logic[1:0]  clk_div;
    logic       turbo_clk, cpu_clk;

    always_ff @(posedge i_clk)
    begin
        clk_div <= clk_div + 1'b1;
    end
    assign  turbo_clk = TURBO_CLK_10 ? i_clk : clk_div[0];

    clk_sel
    u_clk_sel
    (
        .i_clk1     (clk_div[1]),
        .i_clk2     (turbo_clk),
        .i_sel      (turbo_n),
        .o_clk      (cpu_clk)
    );

    logic reset_n_sync_0, reset_n_sync_1;
    logic reset_n;
    always_ff @(posedge i_clk)
        reset_n_sync_0 <= i_reset_n;
    always_ff @(posedge cpu_clk)
    begin
        reset_n_sync_1 <= reset_n_sync_0;
        reset_n <= reset_n_sync_1;
    end

    logic       cpu_m1_n, cpu_mreq_n, cpu_wr_n, cpu_rd_n, cpu_iorq_n;
    logic       cpu_rfsh_n, cpu_halt_n, cpu_busak_n;
    logic[7:0]  cpu_wdata, cpu_rdata;
    logic[15:0] cpu_addr;

    tv80s
    u_cpu
    (
        // Outputs
        .m1_n       (cpu_m1_n),
        .mreq_n     (cpu_mreq_n),
        .iorq_n     (cpu_iorq_n),
        .rd_n       (cpu_rd_n),
        .wr_n       (cpu_wr_n),
        .rfsh_n     (cpu_rfsh_n),
        .halt_n     (cpu_halt_n),
        .busak_n    (cpu_busak_n),
        .A          (cpu_addr),
        .dout       (cpu_wdata),
        // Inputs
        .reset_n    (reset_n),
        .clk        (cpu_clk),
        .wait_n     ('1),
        .int_n      ('1),
        .nmi_n      ('1),
        .busrq_n    ('1),
        .di         (cpu_rdata),
        .cen        ('1)
    );

	logic[4:0]	r_ctrl_f9;
	logic[7:0]	r_ctrl_1a;
	logic[7:0]	r_ctrl_1b;
	logic[7:0]	r_ctrl_1c;
	logic[7:0]	r_ctrl_2a;
	logic[7:0]	r_ctrl_2b;
	logic[7:0]	r_ctrl_2c;

	logic	w_sel_mem;
	logic	w_sel_mem_f8, w_sel_mem_f9, w_sel_mem_fa;

	logic	w_sel_io;
	logic	w_sel_io_f8, w_sel_io_f9, w_sel_io_fa, w_sel_io_fb;
	logic	w_sel_io_00, w_sel_io_01, w_sel_io_02;

	logic	w_sel_f9_write, w_sel_fb_write;
	logic	w_sel_01_write, w_sel_02_write;

	logic	w_erom;
	logic[4:0]	w_ram_page;

    // IO/MEM ports addresses
	assign w_sel_mem = ~(cpu_mreq_n | ~cpu_rfsh_n) & o128_mode;
	assign w_sel_io = ~cpu_iorq_n;

	assign w_sel_mem_f8 = (cpu_addr[15:8] == 8'hf8) & w_sel_mem;
	assign w_sel_mem_f9 = (cpu_addr[15:8] == 8'hf9) & w_sel_mem;
	assign w_sel_mem_fa = (cpu_addr[15:8] == 8'hfa) & w_sel_mem;

	assign w_sel_io_00 = (cpu_addr[7:2] == 6'h00) & w_sel_io;
	assign w_sel_io_01 = (cpu_addr[7:2] == 6'h01) & w_sel_io;
	assign w_sel_io_02 = (cpu_addr[7:2] == 6'h02) & w_sel_io;

	assign w_sel_io_f8 = (cpu_addr[7:0] == 8'hf8) & w_sel_io;
	assign w_sel_io_f9 = (cpu_addr[7:0] == 8'hf9) & w_sel_io;
	assign w_sel_io_fa = (cpu_addr[7:0] == 8'hfa) & w_sel_io;
	assign w_sel_io_fb = (cpu_addr[7:0] == 8'hfb) & w_sel_io;

    // port 01
	assign w_sel_01_write = w_sel_io_01 & (~cpu_wr_n);
	always @(posedge cpu_clk)
	begin
		if (~reset_n)
		begin
			r_ctrl_1a <= '0;
			r_ctrl_1b <= '0;
			r_ctrl_1c <= '0;
		end
		else if (w_sel_io_01 & (cpu_addr[1:0] == 2'b00))
			r_ctrl_1a <= cpu_wdata;
		else if (w_sel_io_01 & (cpu_addr[1:0] == 2'b01))
			r_ctrl_1b <= cpu_wdata;
		else if (w_sel_io_01 & (cpu_addr[1:0] == 2'b10))
			r_ctrl_1c <= cpu_wdata;
	end

	assign ram_page_pro	= r_ctrl_1a;

	assign rom2_addr	= r_ctrl_1b;

	assign ram0_en		= r_ctrl_1c[0];
	assign ram1_en		= r_ctrl_1c[1];
	assign ram2_en		= r_ctrl_1c[2];
	assign rom2_en		= r_ctrl_1c[3];
	assign rom1_en		= r_ctrl_1c[4];
	assign turbo_n		= r_ctrl_1c[5];
	assign top_mem_en	= r_ctrl_1c[6];
	assign o128_mode	= r_ctrl_1c[7];

// port 02
	assign w_sel_02_write = w_sel_io_02 & (~cpu_wr_n);
	always @(posedge cpu_clk)
	begin
		if (~reset_n)
		begin
			r_ctrl_2a <= '0;
			r_ctrl_2b <= '0;
			r_ctrl_2c <= '0;
		end
		else if (w_sel_io_02 & (cpu_addr[1:0] == 2'b00))
			r_ctrl_2a <= cpu_wdata;
		else if (w_sel_io_02 & (cpu_addr[1:0] == 2'b01))
			r_ctrl_2b <= cpu_wdata;
		else if (w_sel_io_02 & (cpu_addr[1:0] == 2'b10))
			r_ctrl_2c <= cpu_wdata;
	end

	assign ram0_page = r_ctrl_2a;
	assign ram1_page = r_ctrl_2b;
	assign ram2_page = r_ctrl_2c;

    // port F9
	always @(posedge cpu_clk)
	begin
		if (~reset_n)
			r_ctrl_f9 <= '0;
		else if ((w_sel_mem_f9 | w_sel_io_f9) & (~cpu_wr_n))
			r_ctrl_f9 <= cpu_wdata[4:0];
	end
	assign w_ram_page = r_ctrl_f9;

    // port FB
	always @(posedge cpu_clk)
	begin
		if (~reset_n)
			ctrl_FB_erom <= '1;
		else if ((w_sel_mem_f8 | w_sel_io_f8) & (~cpu_wr_n))
			ctrl_FB_erom <= '0;
	end
	always @(posedge cpu_clk)
	begin
		if (~reset_n)
        begin
            ctrl_FB_SS <= '0;
            ctrl_FB_BS <= '0;
            ctrl_FB_xmem <= '0;
            ctrl_FB_int <= '0;
            ctrl_FB_mz <= '1;
        end
		else if (w_sel_io_fb & (~cpu_wr_n))
        begin
            ctrl_FB_SS <= cpu_wdata[1:0];
            ctrl_FB_BS <= cpu_wdata[4:2];
            ctrl_FB_xmem <= cpu_wdata[5];
            ctrl_FB_int <= cpu_wdata[6];
            ctrl_FB_mz <= cpu_wdata[7];
        end
	end

    logic ctrl_mz;
	assign w_erom = rom1_en | ctrl_FB_erom;
    //assign intn = !(ctrl_FB_int & frame_end);
    assign ctrl_mz = cpu_addr[15] | cpu_addr[14] | ctrl_FB_mz;

	assign cpu_rdata =	(w_sel_io_00 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b00)) ? i_cfg_sw :
						(w_sel_io_01 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b00)) ? r_ctrl_1a :
						(w_sel_io_01 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b01)) ? r_ctrl_1b :
						(w_sel_io_01 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b10)) ? r_ctrl_1c :
						(w_sel_io_02 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b00)) ? r_ctrl_2a :
						(w_sel_io_02 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b01)) ? r_ctrl_2b :
						(w_sel_io_02 & (!cpu_rd_n) & (cpu_addr[1:0] == 2'b10)) ? r_ctrl_2c :
						((w_sel_mem_f9 | w_sel_io_f9) & (!cpu_rd_n)) ? { 3'b0, r_ctrl_f9 } :
						'z;

    logic mem_fxxx, mem_top;
    logic[2:0] mb;
    logic[1:0] ma_sel;

    assign mem_fxxx = cpu_addr[12] & cpu_addr[13] & cpu_addr[14] & cpu_addr[15] & o128_mode;
    assign mem_top = (cpu_addr[12] & cpu_addr[13] & cpu_addr[14] & cpu_addr[15] & top_mem_en) | mem_fxxx;
    assign mb[0] = !(ram0_en & (!cpu_addr[14]) & (!cpu_addr[15]));	// RAM0
    assign mb[1] = !(ram1_en &   cpu_addr[14]  & (!cpu_addr[15]));	// RAM1
    assign mb[2] = !(ram2_en & (!cpu_addr[14]) &   cpu_addr[15]);	// RAM2
    assign ma_sel[0] = mb[0] & mb[1];
    assign ma_sel[1] = mb[0] & mb[2];

    logic[20:14] mem_addr_hi;
    logic[4:0] ram_page_sel;
    assign ram_page_sel = o128_mode ? w_ram_page : ram_page_pro[4:0];
    assign mem_addr_hi = ((!ctrl_mz) & (!cpu_addr[15]) & (!cpu_addr[14])) ? { 2'b00, ctrl_FB_BS, ctrl_FB_SS }
                       : (mem_top) ? { 5'b1_1111, cpu_addr[15:14] }
                       : (ma_sel == 2'b00) ? r_ctrl_2a[6:0]
                       : (ma_sel == 2'b01) ? r_ctrl_2b[6:0]
                       : (ma_sel == 2'b10) ? r_ctrl_2c[6:0]
                       : { ram_page_sel, cpu_addr[15:14] };

    logic rom_sel, rom1_sel, rom2_sel;
    assign rom_sel  = cpu_iorq_n & (!cpu_addr[15]) & (!cpu_addr[14]);
    assign rom1_sel = rom_sel &    (!cpu_addr[13]) & (rom1_en | ctrl_FB_erom);
    assign rom2_sel = rom_sel &      cpu_addr[13]  &  rom2_en;

    logic mem_blk, mem_wrn, mem_rdn, mem_req;
    assign mem_blk = (((cpu_addr[10] | cpu_addr[11]) & mem_fxxx) & (!ctrl_FB_xmem)) | rom1_sel | rom2_sel;
    assign mem_wrn = cpu_wr_n | cpu_mreq_n | mem_blk;
    assign mem_rdn = cpu_rd_n | cpu_mreq_n | mem_blk;
    assign mem_req = !(mem_wrn & mem_rdn);

    // ROM's
    logic[7:0] rom_1[8192];
    assign cpu_rdata = (rom1_sel & (!cpu_rd_n)) ? rom_1[cpu_addr[12:0]] : 'z;

    // RAM's
    logic[7:0]  ram[1024*1024*2] /* verilator public */;
    logic[20:0] ram_addr;
    logic[7:0]  ram_rdata;

    assign ram_addr = { mem_addr_hi, cpu_addr[13:0] };
    always_ff @(posedge cpu_clk)
    begin
        if (!mem_wrn)
            ram[ram_addr] <= cpu_wdata;
    end
    assign ram_rdata = ram[ram_addr];
    assign cpu_rdata = (!mem_rdn) ? ram_rdata : 'z;

initial
begin
    clk_div = '0;
    $readmemh("../../ROMs/test_pro.hex", rom_1);
end
/* verilator lint_on  UNUSEDSIGNAL */

endmodule