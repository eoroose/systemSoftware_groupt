all:
	gcc -g main.c ma_malloc.c -o main.o -Wall -std=c11 -Werror


run:
	./main.o

test:
	cppcheck --enable=all --suppress=missingIncludeSystem main.c ma_malloc.c

gdb:
	gdb -tui main.o

clean:
	rm -rf *.o