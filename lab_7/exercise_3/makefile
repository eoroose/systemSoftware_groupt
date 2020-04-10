all:
	gcc -Wall -std=c11 -Werror main.c sensor_db.c sensor_db.h -lsqlite3 -o main.o
	gcc file_creator.c -o file_creator.o -Wall -D DEBUG

file_creator:
	./file_creator.o

run: file_creator
	./main.o

test:
	cppcheck --enable=all --suppress=missingIncludeSystem sensor_db.c sensor_db.h
	valgrind --leak-check=full ./main.o

clean:
	rm -rf *.o *.map *.db* sensor_data* *.log
