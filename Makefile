# Makefile

CC=gcc
C_FLAGS=-pedantic -lm -O2 -I.
TARGETS=${subst .c,, ${shell find . -type f -name '*.c'}}

all: ${TARGETS}

%: %.c termgui.h
	${CC} $< -o $@ ${C_FLAGS} ${shell head $< -n 1 | grep -oP '(?<=C_FLAGS: ).*'}

${TARGETS}: C_FLAGS+=-D USE_LOG_FILE

clean:
	rm ${TARGETS}

.PHONY: clean
