# systemSoftware_groupt
Advanced C programming concepts: pointers, dynamic memory, file I/O, system calls, low-level operations, call-backs, function pointers, command-line calls, macros, ...

C build process and tools (preprocessor, compiler, linker, profiler, debugger, Valgrind, objdump, nm, ldd, libraries, ...).

Linux and the Linux system call interface

Concurrency and multi-tasking (threads, processes, synchronization, inter-task communication)

### lab 1
writing basic C programs, build them with gcc, run and test the programs.

### lab 2
pointers in C, parameter passing (call-by-value/reference),understanding the function stack in C, strings in C

### lab 4
understand dynamic memory and what malloc() and free() really do, be able to program pointer manipulations, implement basic memory allocation algorithms, learn to use GDB

### lab 5
understanding dynamic data structures and implementing a double-linked pointer list with void pointer elements and callback functions

### lab 6
learn to create and use static and dynamic libraries, ldd, nm, and objdump; be able to implement code related to file reading and writing.

### lab 7
file I/O, SQL database interfacing, processes and IPC (pipe) communication

### lab 8
programming with processes and TCP/IP socket communication

### lab 9
multi-threading programming and thread-safe data structures

### FINAL
The sensor monitoring system consists of sensor nodes measuring the room temperature, a sensor gateway that acquires all sensor data from the sensor nodes, and an SQL database to store all sensor data processed by the sensor gateway. A sensor node uses a private TCP connection to transfer the sensor data to the sensor gateway. The SQL database is an SQLite system (see lab 7).

The sensor gateway may not assume a maximum amount of sensors at start up. In fact, the number of sensors connecting to the sensor gateway is not constant and may change over time. 

Working with real embedded sensor nodes is not an option for this assignment. Therefore, sensor nodes will be simulated in software (see lab 8). 
