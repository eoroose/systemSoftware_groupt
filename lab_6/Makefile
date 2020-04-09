all: lib_dplist
	gcc main.c datamgr.c lib/dplist.c -o main.o -Wall -std=c11 -Werror -D SET_MAX_TEMP=15 -D SET_MIN_TEMP=20
	gcc file_creator.c -o file_creator.o -Wall -D DEBUG

lib_dplist:
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm 

file_creator:
	./file_creator.o

run: file_creator
	./main.o

test:
	cppcheck --enable=all --suppress=missingIncludeSystem datamgr.c datamgr.h config.h
	valgrind --leak-check=full ./main.o

clean:
	rm -rf *.o lib/*.so lib/*.o *.map sensor_data*

