# Makefile

CC=gcc
C_FLAGS=-pedantic -lm -O2 -I.
TARGETS=test utf8code grid edit

all: ${TARGETS}

test: examples/test.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}
utf8code: utf8code.c
	${CC} $< -o $@ ${C_FLAGS}
grid: examples/grid.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}
edit: examples/edit.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}

clean:
	rm ${TARGETS}

.PHONY: clean
