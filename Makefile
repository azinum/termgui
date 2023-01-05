# Makefile

CC=gcc
C_FLAGS=-pedantic -lm -O2

all: main utf8code grid edit

main: main.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}
utf8code: utf8code.c
	${CC} $< -o $@ ${C_FLAGS}
grid: grid.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}
edit: edit.c termgui.h
	${CC} $< -o $@ ${C_FLAGS}

.PHONY:
