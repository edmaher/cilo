# Configuration for the Cisco 3620/3640 Routers
# TARGET=c3600
# MACHCODE=0x1e
# TEXTADDR=0x80008000
# LOADADDR=0x80028000
# #ifndef CROSS_COMPILE
# CROSS_COMPILE=mips-linux-gnu-
# ARCH=-march=mips32
# #endif
# CFLAGS=-mno-abicalls
# LDFLAGS=-Ttext ${TEXTADDR} -Wl,--script=mips.ld

# Configuration for the Cisco 3660 Routers
# TARGET=c3600
# MACHCODE=0x34
# TEXTADDR=0x80008000
# LOADADDR=0x80028000
# ifndef CROSS_COMPILE
# CROSS_COMPILE=mips-linux-gnu-
# ARCH=-march=mips32
# endif
# CFLAGS=-mno-abicalls
# LDFLAGS=-Ttext ${TEXTADDR}

# Configuration for the Cisco 1700 Series Routers
# TARGET=c1700
# MACHCODE=0x33
# TEXTADDR=0x80008000
# LOADADDR=0x80028000
# #ifndef CROSS_COMPILE
# CROSS_COMPILE=powerpc-linux-gnu-
# ARCH=-mcpu=860 # based on 1721
# #endif
# LDFLAGS=-Ttext=${TEXTADDR}

# Configuration for the Cisco 1800 Series Routers
TARGET=c1800
MACHCODE=0x93 #147
TEXTADDR=0x80012000
LOADADDR=0x80012000
#ifndef CROSS_COMPILE
CROSS_COMPILE=powerpc-linux-gnu-
ARCH=-mcpu=powerpc # based on 1811
#endif
CFLAGS=-fno-exceptions -fno-unwind-tables -fomit-frame-pointer -fno-asynchronous-unwind-tables
LDFLAGS=-Ttext=${TEXTADDR} -fno-exceptions -fno-unwind-tables -Wl,--build-id=none -z nodefaultlib -nostartfiles

# -z noexecstack
#
# notes: there is an addtional entry "GNU_STACK" in the Program Headers
# in the code, it's type PT_GNU_STACK
# in the bfd code, it appears that there is no *obvious* way to pass an option to bfd from the linker to suppress the
# creation of the PT_GNU_STACK
#  try removing the .GNU.stack section in object files
#need s no no execstack
# eecstack utility allows the flag to be fillped bit not reemoved


# Configuration for the Cisco 7200 Series Routers
# TARGET=c7200
# MACHCODE=0x19
# TEXTADDR=0x80008000
# LOADADDR=0x80028000
# ifndef CROSS_COMPILE
# CROSS_COMPILE=mips-linux-gnu-
# endif
# CFLAGS=-DDEBUG -mno-abicalls
# LDFLAGS=-Ttext ${TEXTADDR}

# additional CFLAGS
CFLAGS+=

# don't modify anything below here
# ===================================================================

PROG=ciscoload

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

MACHDIR=mach/$(TARGET)

# command to prepare a binary
RAW=${OBJCOPY} --strip-unneeded --alt-machine-code ${MACHCODE}

INCLUDE=-Iinclude/ -Imach/${TARGET} -Iinclude/mach/${TARGET}

CFLAGS+=-fno-builtin -fomit-frame-pointer -fno-pic \
	-fno-stack-protector -Wall ${ARCH} -DLOADADDR=${LOADADDR}

ASFLAGS=-D__ASSEMBLY__-xassembler-with-cpp -traditional-cpp

## re-add -Wl,--strip-all for release, leave for dev
LDFLAGS+=-Wl,--omagic -nostartfiles -nostdlib -Wl,--discard-all \
	--entry _start ${ARCH}

OBJECTS=string.o main.o ciloio.o printf.o elf_loader.o lzma_loader.o \
	LzmaDecode.o

LINKOBJ=${OBJECTS} $(MACHDIR)/promlib.o $(MACHDIR)/start.o $(MACHDIR)/platio.o\
	$(MACHDIR)/platform.o


THISFLAGS='LDFLAGS=$(LDFLAGS)' 'ASFLAGS=$(ASFLAGS)' \
	'CROSS_COMPILE=$(CROSS_COMPILE)' 'CFLAGS=$(CFLAGS)' 'CC=$(CC)'

all: ${OBJECTS} ${PROG}

${PROG}: sub ${OBJECTS}
	${CC} ${LDFLAGS} ${LINKOBJ} -o ${PROG}.elf
	${RAW} ${PROG}.elf ${PROG}.bin

.c.o:
	${CC} ${CFLAGS} $(INCLUDE) -c $< -o $@.x
	${OBJCOPY} --remove-section .note.GNU-stack $@.x $@
	rm -f $@.x

.S.o:
	${CC} ${CFLAGS} $(INCLUDE) ${ASFLAGS} -c $< -o $@.x
	${OBJCOPY} --remove-section .note.GNU-stack $@.x $@
	rm -f $@.x

sub:
	@for i in $(MACHDIR); do \
	echo "Making all in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) $(THISFLAGS) all); done

subclean:
	@for i in $(MACHDIR); do \
	echo "Cleaning all in $$i..."; \
	(cd $$i; $(MAKE) $(MFLAGS) clean); done

clean: subclean
	-rm -f *.o
	-rm -f *.o.x
	-rm -f ${PROG}.elf
	-rm -f ${PROG}.bin
