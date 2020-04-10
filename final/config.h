/*
 * interpretation of my DEBUG and ERR_HANDLER code is derived from lab 5 dplist debugger and lab 8 tcpsock debugger.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    // force macro-expansion on s before stringify s

#ifdef DEBUG
	#define DEBUG_PRINTF(...) 									                        		                  \
		do {											                                                              \
		    fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	\
		    fprintf(stderr,__VA_ARGS__);								                   		                  \
		    fflush(stderr);                                                                     \
        } while(0);
#else
	#define DEBUG_PRINTF(...) (void)0
#endif

#define ERR_HANDLER(condition, err_code)                                                        \
	do {						                                                                              \
        if ((condition)) DEBUG_PRINTF(#condition " failed, error condition: " #err_code "\n");	\
        assert(!(condition));                                                                   \
    } while(0);

// for datamgr
#ifndef RUN_AVG_LENGTH
  #define RUN_AVG_LENGTH 5 // runninng avg
#endif

// for sensor_db
#ifndef DB_NAME
  #define DB_NAME Sensor.db
#endif

#ifndef TABLE_NAME
  #define TABLE_NAME SensorData
#endif

// for sbuffer
#ifndef READERS
  #define READERS 2	// "READERS" is the amount of threads that will be reading off of the sbuffer, deafult is 2 for datamgr and strgmgr 
#endif

// for main
#ifndef FIFO_PIPE
  #define FIFO_PIPE logFifo // name of fifo pipe
#endif

#ifndef GATEWAY
  #define GATEWAY gateway.log // name of log file
#endif

#ifndef ROOM_SENSOR_MAP
  #define ROOM_SENSOR_MAP room_sensor.map // name of map file
#endif

#ifndef EXIT_PIPE_CONDITION
  #define EXIT_PIPE_CONDITION exiting_pipe // command to to exit FIFO pipe, (like a password)
#endif

#ifndef DB_CONNECTION_TRIES
  #define DB_CONNECTION_TRIES	3 // amount of times db is allowed to disconnect
#endif

#ifdef DB_FAIL    // if -DDB_FAIL, then the connection to db will fail, just for testing purposes
  #define CONFIG_DISCONNECT_DB 0  
# else
  #define CONFIG_DISCONNECT_DB 1  
#endif

#define CONFIG_DISCONNECT_TRUE  0   // to check condition of "CONFIG_DISCONNECT_DB"
#define CONFIG_DISCONNECT_FALSE 1

#define CONFIG_THREAD_GET_DATA  0
#define CONFIG_THREAD_WAIT      1
#define CONFIG_THREAD_TERMINATE 2

#define CONFIG_FIFO_SIZE 500

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;     
typedef time_t sensor_ts_t;		// UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct{
	sensor_id_t id;
	sensor_value_t value;
	sensor_ts_t ts;
} sensor_data_t;

#endif /* _CONFIG_H_ */

