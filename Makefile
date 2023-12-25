default: all

sim:
	make -C $(MAKECMDGOALS) $(target)

clean:
	@echo ">>> Clean all <<<"
	make -C sim clean

all: clean

.PHONY: sim clean
$(V).SILENT:
