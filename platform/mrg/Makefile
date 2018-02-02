include ../../global.mk
include ${BUILDRUMP_TOOLFLAGS}

default: all

OBJ_DIR ?= $(CURDIR)/obj

LDSCRIPT:= $(PWD)/i386murgia.x
SRCS+= main.c bmk_mrg.c user_mrg.c

CPPFLAGS+= -isystem ~/mh/dist/usr/include

MRG_LDFLAGS+= -L ~/mh/dist/lib -L ~/mh/dist/usr/lib -L ~/mh/toolchain/install/i686-elf-murgia/lib


include ../Makefile.inc

OBJS:=	$(patsubst %.c,${RROBJ}/%.o,${SRCS}) \
	$(patsubst %.S,${RROBJ}/%.o,${ASMS})

all: links ${MAINOBJ} ${TARGETS}

links:
	[ -e ../../include/bmk-pcpu ] || ln -sf ../platform/mrg/include/ ../../include/bmk-pcpu

${RROBJ}/main.o: main.c
	${CC} -c $< -o $@

${RROBJ}/%.o: %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

${MAINOBJ}: ${OBJS} platformlibs
	${LD} -nostdlib -r ~/mh/dist/lib/crt0.o ${OBJS} -o $@ \
	    -L${RROBJLIB}/libbmk_core -L${RROBJLIB}/libbmk_rumpuser \
	    --whole-archive -lbmk_rumpuser -lbmk_core --no-whole-archive \
	    ${MRG_LDFLAGS} -lmrg -lsquoze -luk -lc -lmrg -lc -luk
	objcopy -w -G "bmk_*" -G "rumpuser_*" -G "jsmn_*" \
	    -G rumprun_platform_rumpuser_init -G _start $@
