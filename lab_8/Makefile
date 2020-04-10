TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

all: lib_dplist lib_tcpsock sensor_node
	gcc main.c connmgr.c config.h lib/tcpsock.c lib/dplist.c -DTIMEOUT=5 -o main.o

sensor_node:
	gcc -c sensor_node.c -Wall -std=c11 -Werror -o sensor_node.o -fdiagnostics-color=auto
	gcc sensor_node.o -ltcpsock -o sensor_node -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto
	
lib_dplist:
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o -fdiagnostics-color=auto
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm -fdiagnostics-color=auto

lib_tcpsock:
	gcc -c lib/tcpsock.c -Wall -std=c11 -Werror -fPIC -o lib/tcpsock.o -fdiagnostics-color=auto
	gcc lib/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm -fdiagnostics-color=auto

sensors:
	@echo "$(TITLE_COLOR)\n***** five sensors running *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 2 127.0.0.1 5678 &
	./sensor_node 37 3 127.0.0.1 5678 &
	./sensor_node 49 4 127.0.0.1 5678 &
	./sensor_node 112 5 127.0.0.1 5678 &
	sleep 30
	pkill sensor_node -f -e -c

run:
	gnome-terminal -- make sensors
	./main.o 5678

test:
	valgrind -s --leak-check=full ./main.o

clean:
	rm -rf *.o lib/*.o lib/*.so sensor_data_recv sensor_node