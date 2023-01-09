# Makefile

CC=gcc
C_FLAGS=-pedantic -lm -O2 -I.
TARGETS=${subst .c,, ${shell find . -type f -name '*.c'}}

all: ${TARGETS}

%: %.c termgui.h
	${CC} $< -o $@ ${C_FLAGS} ${shell head $< -n 1 | grep -oP '(?<=C_FLAGS: ).*'}

clean:
	rm ${TARGETS}

.PHONY: clean
