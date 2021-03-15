# Makefile for first lex example
CC = gcc 
LEX = flex
CFLAGS = -g -Wall

src/scanner.c: src/lex.l
	$(LEX) -t  $< > $@

wish: src/scanner.c
	cc $(CFLAGS) $< -o $@



.PHONY: clean testfiles
