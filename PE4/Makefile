# Makefile for first lex example
CC = gcc 
LEX = flex
CFLAGS = -g -Wall

wish: src/scanner.c
	$(CC) $(CFLAGS) $< -o $@

src/scanner.c: src/lex.l
	$(LEX) -t  $< > $@
