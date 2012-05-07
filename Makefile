#
# Makefile for Taks-Timing
#

CC=gcc
CFLAGS=-Wall -g -pedantic -ansi

all:		prog

prog: main.c
		$(CC) $(CFLAGS) -o main main.c

clean:
		rm main

debug:
		gdb main
