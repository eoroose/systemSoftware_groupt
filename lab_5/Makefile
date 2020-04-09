all:
	gcc main.c dplist.c -o main.o -Wall -std=c11 -Werror

run:
	./main.o

test:
	cppcheck --enable=all --suppress=missingIncludeSystem main.c dplist.c dplist.h
	valgrind --leak-check=full ./main.o

clean:
	rm -rf *.o