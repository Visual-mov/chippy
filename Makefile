.SUFFIXES: .c .o

CC = gcc
CCFLAGS = -std=c99 -pedantic -Wall # -g
LFLAGS = -lSDL2 -lm

EXEC = chippy
OBJS = chippy.o chip8.o

${EXEC}: ${OBJS}
	${CC} ${CCFLAGS} -o ${EXEC} ${OBJS} ${LFLAGS}

.c.o:
	${CC} ${CCFLAGS} -c $<

run: ${EXEC}
	./${EXEC}

# valgrind: ${EXEC}
# 	valgrind --leak-check=full ./${EXEC}

# gdb: ${EXEC}
# 	gdb ./${EXEC}

clean:
	rm -f ${EXEC} ${OBJS}

chippy.o: chippy.c chippy.h common.h
chip8.o: chip8.c chip8.h common.h