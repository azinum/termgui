#!/usr/bin/env sh

set -xe

gcc main.c -o main -O2 -pedantic
gcc utf8code.c -o utf8code -Wall -O2 -pedantic
