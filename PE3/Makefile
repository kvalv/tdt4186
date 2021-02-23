FLAGS = -Wall
# MEM_SIZE ?= 65536
MEM_SIZE ?= 192

# FLAGS = -Wall -D"MEM_SIZE=192"
prg: mymalloc.c
	@$(CC) $(FLAGS) -D"MEM_SIZE=$(MEM_SIZE)" $< -o $@

run: prg
	./prg

all-tests: mymalloc.c
	./run-tests tests/*

.PHONY: run prg all-tests
