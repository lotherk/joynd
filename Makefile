PREFIX := /usr/local
MANDIR := ${PREFIX}/man/man1
CFLAGS := -g `pkg-config --cflags x11 --libs x11 --libs xtst --libs sdl2 --cflags sdl2`

default: all

all: program

opts: cmdline
cmdline:
	gengetopt -u -i joy2key.ggo

program:
	cc ${CFLAGS} -o joy2key joy2key.c cmdline.c
readme.md: all
	erb -T- ./README.md.erb > README.md
clean:
	-rm -f joy2key

install:
	install -d ${PREFIX}/bin
	install -m 755 joy2key ${PREFIX}/bin/
	install -d ${MANDIR}
	install -m 644 joy2key.1  ${MANDIR}/

deinstall:
	-rm -f ${PREFIX}/bin/joy2key
	-rm -f ${MANDIR}/joy2key.1

