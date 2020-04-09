all:
	gcc exercise_1_1.c -o exercise_1_1.o
	gcc exercise_1_2.c -o exercise_1_2.o
	gcc exercise_2.c -o exercise_2.o
	gcc main.c -o main.o -Wall -std=c11 -Werror

run_ex_11:
	./exercise_1_1.o
run_ex_12:
	./exercise_1_2.o
run_ex_2:
	./exercise_2.o
run_main:
	./main.o

test_main:
	cppcheck main.c --enable=all --suppress=missingIncludeSystem ./main.o
clean:
	rm -rf *.o