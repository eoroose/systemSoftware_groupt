all:
	gcc main.c config.h sbuffer.c -pthread -o main.o -Wall -std=c11 
	gcc file_creator.c -o file_creator.o -Wall -D DEBUG

file_creator:
	./file_creator.o

run: file_creator
	./main.o sensor_data

clean:
	rm -rf *.o *.txt *.map sensor_data*