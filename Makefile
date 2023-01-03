# Makefile

CC=gcc
C_FLAGS=-pedantic -ggdb -lm

all: main utf8code grid

main: main.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}
utf8code: utf8code.c
	${CC} $< -o $@ ${C_FLAGS}
grid: grid.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}

.PHONY:
