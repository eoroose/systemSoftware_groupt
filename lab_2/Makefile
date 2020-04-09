all:
	gcc exercise_1.c -o exercise_1.o
	gcc exercise_2.c -o exercise_2.o
	gcc exercise_3.c -o exercise_3.o
	gcc main.c -o main.o -Wall -std=c11 -Werror

run_ex1:
	./exercise_1.o
run_ex2:
	./exercise_2.o
run_ex3:
	./exercise_3.o
run_main:
	./main.o

test_main:
	cppcheck main.c --enable=all --suppress=missingIncludeSystem ./main.o
clean:
	rm -rf *.o