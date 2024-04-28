CC = gcc
FLAGS = -Wall -Wextra -fanalyzer
INCLUDES = util.h
SOURCES = filebot.c util.c
ASMSOURCES =
OBJFILES = filebot.o util.o
EXEC = filebot

# Suffix rules
.SUFFIXES : .c .s .o

# How to build an object .o from a code file .c ; $< -- file name
.c.o:
	${CC} ${FLAGS} -c $<

# How to build an object .o from a code file .s 
.s.o:
	${CC} ${FLAGS} -c $<

${EXEC}: ${OBJFILES}
	${CC} ${OBJFILES} -o ${EXEC}

${OBJFILES}: ${SOURCES} ${ASMSOURCES} ${INCLUDES}

run: ${EXEC}
	./${EXEC} filebot.conf

clean:
	rm -f ${OBJFILES} *.o ${EXEC}
