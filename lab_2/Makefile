all:
	gcc main.c -o main -Wall -std=c11 -Werror
run:
	./main
test:
	cppcheck main.c --enable=all --suppress=missingIncludeSystem ./main