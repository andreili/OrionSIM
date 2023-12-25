GTK_FLAGS = --rcfile=../gtkwaverc

all:
	make -C run -f ../Makefile.main GTK_FLAGS=$(GTK_FLAGS) emulate $(MAKECMDGOALS)

clean:
	make -C run -f ../Makefile.main clean

$(V).SILENT:
