#ifndef __CONNMGR_H__
#define __CONNMGR_H__
#define _GNU_SOURCE

#ifndef TIMEOUT
  #error TIMEOUT not specified!(in seconds)
#endif

#define ERROR_HANDLER(condition,...) 	do { \
					  if (condition) { \
					    printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
					    exit(EXIT_FAILURE); \
					  }	\
					} while(0)

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "config.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h" 

void connmgr_listen(int port_number);
/*
This method holds the core functionality of your connmgr.
It starts listening on the given port and when when a sensor node connects it writes the data to a sensor_data_recv file.
This file must have the same format as the sensor_data file in assignment 6 and 7.
*/

void connmgr_free(void);
/*
This method should be called to clean up the connmgr, and to free all used memory.
After this no new connections will be accepted
*/

#endif