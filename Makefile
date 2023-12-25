default: all

sim:
	make -C $(MAKECMDGOALS) $(target)

clean:
	@echo ">>> Clean all <<<"
	make -C sim clean

all: clean

test:
	make -C ROMs/Sjasm
	./ROMs/Sjasm/sjasm -v ./ROMs/test.asm ./ROMs/test.bin ./ROMs/test.lst
	objcopy --verilog-data-width=1 -I binary -O verilog ./ROMs/test.bin ./ROMs/test.hex

.PHONY: sim clean
$(V).SILENT:
