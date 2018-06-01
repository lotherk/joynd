PREFIX := /usr/local
MANDIR := ${PREFIX}/man/man1
CFLAGS := -g `pkg-config --cflags x11 --libs x11 --libs xtst --libs sdl2 --cflags sdl2`

default: all

all: program

opts: cmdline
cmdline:
	gengetopt -i joynd.ggo

program:
	cc ${CFLAGS} -o joynd joynd.c cmdline.c
readme.md: all
	erb -T- ./README.md.erb > README.md
clean:
	-rm -f joynd

install:
	install -d ${PREFIX}/bin
	install -m 755 joynd ${PREFIX}/bin/
	install -d ${MANDIR}
	install -m 644 joynd.1  ${MANDIR}/

deinstall:
	-rm -f ${PREFIX}/bin/joynd
	-rm -f ${MANDIR}/joynd.1

