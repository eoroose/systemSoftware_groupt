all:
	gcc main.c ma_malloc.c -o main.o

run:
	./main.o

test:
	cppcheck main.c ma_malloc.c --enable=all --suppress=missingIncludeSystem ./main.o

clean:
	rm -rf *.o