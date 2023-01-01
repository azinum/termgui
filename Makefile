# Makefile

CC=gcc
C_FLAGS=-pedantic -ggdb

all: main

main: main.c
	${CC} $< -o $@ ${C_FLAGS}
utf8code: utf8code.c
	${CC} $< -o $@ ${C_FLAGS}
 
.PHONY: main
