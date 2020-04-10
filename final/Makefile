
TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

# when executing make, compile all exe's
all: sensor_gateway sensor_node

# When trying to compile one of the executables, first look for its .c files
# Then check if the libraries are in the lib folder
sensor_gateway : file_creator main.c connmgr.c datamgr.c sensor_db.c sbuffer.c lib/libdplist.so lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** CPPCHECK *****$(NO_COLOR)"
	cppcheck --enable=all --suppress=missingIncludeSystem main.c connmgr.c datamgr.c sensor_db.c sbuffer.c
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_gateway *****$(NO_COLOR)"
	gcc -c main.c      -Wall -std=c11 -Werror -DDEBUG 			-DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o main.o      	 -fdiagnostics-color=auto
	gcc -c connmgr.c   -Wall -std=c11 -Werror -DDEBUG 			-DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o connmgr.o   	 -fdiagnostics-color=auto
	gcc -c datamgr.c   -Wall -std=c11 -Werror -DDEBUG 			-DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o datamgr.o   	 -fdiagnostics-color=auto
	gcc -c sensor_db.c -Wall -std=c11 -Werror -DDEBUG 			-DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sensor_db.o 	 -fdiagnostics-color=auto
	gcc -c sbuffer.c   -Wall -std=c11 -Werror -DDEBUG 			-DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sbuffer.o   	 -fdiagnostics-color=auto
	gcc -c main.c      -Wall -std=c11 -Werror -DDEBUG -DDB_FAIL -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o main_dbfail.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_gateway *****$(NO_COLOR)"
	gcc main.o 		  connmgr.o datamgr.o sensor_db.o sbuffer.o -ldplist -ltcpsock -lpthread -o sensor_gateway 		  -Wall -L./lib -Wl,-rpath=./lib -lsqlite3 -fdiagnostics-color=auto
	gcc main_dbfail.o connmgr.o datamgr.o sensor_db.o sbuffer.o -ldplist -ltcpsock -lpthread -o sensor_gateway_dbfail -Wall -L./lib -Wl,-rpath=./lib -lsqlite3 -fdiagnostics-color=auto


file_creator : file_creator.c
	@echo "$(TITLE_COLOR)\n***** COMPILE & LINKING file_creator *****$(NO_COLOR)"
	gcc file_creator.c -o file_creator.o -Wall -fdiagnostics-color=auto

get_rooms_sensors:
	./file_creator.o

sensor_node : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c -Wall -std=c11 -Werror -o sensor_node.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node.o -ltcpsock -o sensor_node -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

# If you only want to compile one of the libs, this target will match (e.g. make liblist)
libdplist : lib/libdplist.so
libsbuffer : lib/libsbuffer.so
libtcpsock : lib/libtcpsock.so

lib/libdplist.so : lib/dplist.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB dplist *****$(NO_COLOR)"
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB dplist< *****$(NO_COLOR)"
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libsbuffer.so : lib/sbuffer.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB sbuffer *****$(NO_COLOR)"
	gcc -c lib/sbuffer.c -Wall -std=c11 -Werror -fPIC -o lib/sbuffer.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB sbuffer *****$(NO_COLOR)"
	gcc lib/sbuffer.o -o lib/libsbuffer.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libtcpsock.so : lib/tcpsock.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB tcpsock *****$(NO_COLOR)"
	gcc -c lib/tcpsock.c -Wall -std=c11 -Werror -fPIC -o lib/tcpsock.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB tcpsock *****$(NO_COLOR)"
	gcc lib/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm -fdiagnostics-color=auto

# do not look for files called clean, clean-all or this will be always a target
.PHONY : clean clean-all 

clean:
	rm -rf logFifo main_dbfail.o sensor_data sensor_gateway sensor_gateway_dbfail sensor_node file_creator *~

clean-all: clean
	rm -rf lib/*.so lib/*o *.o *.log *.map *.db

# sensor configs are not to be ran by the user

sensor_config_1:
	@echo "$(TITLE_COLOR)\n***** used only with scalability_test_1 *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 2 127.0.0.1 5678 &
	./sensor_node 37 3 127.0.0.1 5678 &
	./sensor_node 49 4 127.0.0.1 5678 &
	./sensor_node 112 5 127.0.0.1 5678 &
	sleep 30
	pkill sensor_node -f -e -c

sensor_config_2:
	@echo "$(TITLE_COLOR)\n***** used only with scalability_test_2 *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 2 127.0.0.1 5678 &
	./sensor_node 37 3 127.0.0.1 5678 &
	./sensor_node 49 4 127.0.0.1 5678 &
	./sensor_node 112 5 127.0.0.1 5678 &
	sleep 30
	pkill sensor_node -f -o -e
	sleep 10
	pkill sensor_node -f -o -e
	sleep 10
	pkill sensor_node -f -o -e
	sleep 10
	pkill sensor_node -f -o -e
	sleep 10
	pkill sensor_node -f -o -e

sensor_config_3:
	@echo "$(TITLE_COLOR)\n***** used only with stress_test *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 1 127.0.0.1 5678 &
	./sensor_node 37 1 127.0.0.1 5678 &
	./sensor_node 49 1 127.0.0.1 5678 &
	./sensor_node 112 1 127.0.0.1 5678 &
	sleep 30
	pkill sensor_node -f -e -c

sensor_config_4:
	@echo "$(TITLE_COLOR)\n***** used only with durability_test *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 2 127.0.0.1 5678 &
	./sensor_node 37 3 127.0.0.1 5678 &
	./sensor_node 49 4 127.0.0.1 5678 &
	./sensor_node 112 5 127.0.0.1 5678 &
	sleep 3600
	pkill sensor_node -f -e -c

sensor_config_5:
	@echo "$(TITLE_COLOR)\n***** used only with db_fail_test *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 1 127.0.0.1 5678 &
	./sensor_node 21 2 127.0.0.1 5678 &
	./sensor_node 37 3 127.0.0.1 5678 &
	./sensor_node 49 4 127.0.0.1 5678 &
	./sensor_node 112 5 127.0.0.1 5678 &
	sleep 10
	pkill sensor_node -f -e -c

sensor_config_6:
	@echo "$(TITLE_COLOR)\n***** used only with valgrind_test *****$(NO_COLOR)"
	sleep 2
	./sensor_node 15 5 127.0.0.1 5678 &
	./sensor_node 21 5 127.0.0.1 5678 &
	./sensor_node 37 5 127.0.0.1 5678 &
	sleep 8
	pkill sensor_node -f -e -c

# runnable tests NOTE: all run with the DEBUG flag on

scalability_test_1: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_1
	./sensor_gateway 5678
	make clean

scalability_test_2: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_2
	./sensor_gateway 5678
	make clean

stress_test: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_3
	./sensor_gateway 5678
	make clean

durability_test: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_4
	./sensor_gateway 5678
	make clean

db_fail_test: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_5
	./sensor_gateway_dbfail 5678
	make clean

valgrind_test: sensor_gateway get_rooms_sensors sensor_node
	gnome-terminal -- make sensor_config_6
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./sensor_gateway 5678
	make clean

