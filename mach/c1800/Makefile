ifndef CROSS_COMPILE
CROSS_COMPILE=powerpc-elf-
endif

OBJECTS=start.o promlib.o platform.o platio.o

INCLUDE=-I../../include
OBJCOPY=$(CROSS_COMPILE)objcopy

all: ${OBJECTS}

.c.o:
	${CC} ${CFLAGS} $(INCLUDE) -c $< -o $@.x
	${OBJCOPY} --remove-section .note.GNU-stack $@.x $@


.S.o:
	${CC} ${CFLAGS} $(INCLUDE) ${ASFLAGS} -c $< -o $@.x
	${OBJCOPY} --remove-section .note.GNU-stack $@.x $@

clean:
	-rm -f *.o
