#
# Makefile for Taks-Timing
#

CC=gcc
CFLAGS=-Wall -pedantic -ansi

all:		prog

prog: main.c
		$(CC) $(CFLAGS) -o main main.c

clean:
		rm main

