# BSD make compatible build script for NetBSD
CC?=cc
CFLAGS+=-O2 -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L -D_NETBSD_SOURCE -Iinclude

SOURCES=src/main.c src/cli_parser.c src/file_utils.c src/scanner.c src/ordering.c src/formatter.c
OBJECTS=${SOURCES:.c=.o}
TARGET=new_ls
PREFIX?=/usr/local
BINDIR=${PREFIX}/bin

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CC} ${OBJECTS} -o ${TARGET}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

test: ${TARGET}
	sh tests/test.sh

install: ${TARGET}
	install -d ${DESTDIR}${BINDIR}
	install -m 755 ${TARGET} ${DESTDIR}${BINDIR}/${TARGET}

uninstall:
	rm -f ${DESTDIR}${BINDIR}/${TARGET}

clean:
	rm -f ${OBJECTS} ${TARGET}

.PHONY: all test install uninstall clean
