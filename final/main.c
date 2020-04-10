/*
 * interpretation of my pthread code is derived from "pthread Tutorial" by Peter C. Chapin (PDF from Toledo)
 * 
 * interpretation of my FIFO code is derived from the web page "tutorialpoints.dev" on an article called "Named Pipe or FIFO with example C program"
 * URL: https://tutorialspoint.dev/computer-science/operating-systems/named-pipe-fifo-example-c-program
 * 
 * interpreation of my FORK code id derived from the webpage "geekforgeeks.com" on an aricle called "fork() in C"
 * URL: https://www.geeksforgeeks.org/fork-system-call/
 * 
 * interpretation of my callback functions code is derived from the book "understanding and Using C pointers" by Richard Reese, chapter 3 "Pointers and Functions" 
 * 
 * other citings can be found in "dplist.h", "connmgr.h" and "config.h"
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <signal.h>

#include <string.h> 
#include <fcntl.h> 
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"

#define MAIN_PTHREAD_SUCCESS 0
#define MAIN_PTHREAD_FAILURE 1
#define MAIN_INVALID_ERROR   2
#define MAIN_FILE_ERROR		 3
#define MAIN_FORK_FAILURE	 4				

sbuffer_t *sensor_buffer;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
FILE *fifo_write, *fifo_read;
bool flag = false;	// when connmgr terminates, the flag will alert the other threads to terminate as soon as theyre done getting all the data from the buffer

// callback for writing into the sbuffer
void write_to_buffer(sensor_data_t *data) {
	ERR_HANDLER(data == NULL, MAIN_INVALID_ERROR);
	pthread_mutex_lock(&mutex);
	sbuffer_insert(sensor_buffer, data);
	pthread_mutex_unlock(&mutex);
}

// callback for reading from the sbuffer, returns a message, of what the thread should do
int read_from_buffer(sensor_data_t *data, int id) {
	ERR_HANDLER(data == NULL, MAIN_INVALID_ERROR);
	ERR_HANDLER(id < 0, MAIN_INVALID_ERROR);
	int message = CONFIG_THREAD_GET_DATA;

	pthread_mutex_lock(&mutex);
	if(sbuffer_remove(sensor_buffer, data, id) == SBUFFER_NO_DATA) {
		if(flag == true) message = CONFIG_THREAD_TERMINATE;
		else message = CONFIG_THREAD_WAIT;
	}
	pthread_mutex_unlock(&mutex);
	return message;
}

// callback function to write into the fifo pipe
void write_to_fifo(char *fifo_buffer) {
	pthread_mutex_lock(&mutex);
	fprintf(fifo_write, "%s", fifo_buffer);
	pthread_mutex_unlock(&mutex);
}

// function to read from the fifo pipe, for the child process
void read_from_fifo(FILE *gateway) {
	ERR_HANDLER(gateway == NULL, MAIN_INVALID_ERROR);
	char fifo_buffer[CONFIG_FIFO_SIZE];
    int sequence = 0;

	while(fgets(fifo_buffer, CONFIG_FIFO_SIZE, (FILE*)fifo_read) != NULL) {
    	if(strcmp(fifo_buffer, TO_STRING(EXIT_PIPE_CONDITION)) == 0) break;
    	fprintf(gateway, "%d: time: %ld  message: %s", ++sequence, time(NULL), fifo_buffer);
    }
}

// initialize fifo for writing, for the parent process
void init_fifo_write() {
	mkfifo(TO_STRING(FIFO_PIPE), 0666);
	fifo_write = fopen(TO_STRING(FIFO_PIPE), "w");

	if(fifo_write == NULL) DEBUG_PRINTF("error opening fifo (write)\n");
	ERR_HANDLER(fifo_write == NULL, MAIN_FILE_ERROR);
}

// initialize fifo for reading, for the child process
void init_fifo_read() {
	mkfifo(TO_STRING(FIFO_PIPE), 0666);
    fifo_read = fopen(TO_STRING(FIFO_PIPE), "r");
    
	if(fifo_read == NULL) DEBUG_PRINTF("error opening fifo (read)\n");
	ERR_HANDLER(fifo_read == NULL, MAIN_FILE_ERROR);
}

// function for the connmgr thread
void *connmgr(void *arg) {
	ERR_HANDLER(arg == NULL, MAIN_INVALID_ERROR);
	int port = *(int*)arg;	

	connmgr_listen_buffer(port, write_to_buffer, write_to_fifo);
	connmgr_free();

	pthread_mutex_lock(&mutex);
	flag = true;
	pthread_mutex_unlock(&mutex);

	DEBUG_PRINTF("finished connmgr\n");
	pthread_barrier_wait(&barrier);
	return MAIN_PTHREAD_SUCCESS;
} 

// function for the datamgr thread
void *datamgr(void *arg) {
	ERR_HANDLER(arg == NULL, MAIN_INVALID_ERROR);
	int id = *(int*)arg;

	FILE *map_file =fopen(TO_STRING(ROOM_SENSOR_MAP), "r");
	ERR_HANDLER(map_file == NULL, MAIN_FILE_ERROR);

	datamgr_parse_sensor_buffer(map_file, read_from_buffer, id, write_to_fifo);
	datamgr_free();

	fclose(map_file);
	DEBUG_PRINTF("finished datamgr\n");

	pthread_barrier_wait(&barrier);
	return MAIN_PTHREAD_SUCCESS;
}

// function for the strgmgr thread
void *strgmgr(void *arg) {
	ERR_HANDLER(arg == NULL, MAIN_INVALID_ERROR);
	int tries, error_code, id = *(int*)arg;
	
	char clear_up_flag = STRGMGR_CLEAR_TABLE_TRUE, fifo_buffer[CONFIG_FIFO_SIZE];
	DBCONN *db = NULL;

	for(tries = 1; tries <= DB_CONNECTION_TRIES; tries++) {
		// ifdef DB_FAIL then we are testing for program exiting after db disconnection
		if(CONFIG_DISCONNECT_DB == CONFIG_DISCONNECT_FALSE) db = init_connection(clear_up_flag, write_to_fifo);

		if(db == NULL) {
			snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "Unable to connect to SQL server\n");
			write_to_fifo(fifo_buffer);
		}
		else  {
			clear_up_flag = STRGMGR_CLEAR_TABLE_FALSE;
			error_code = insert_sensor_from_buffer(db, read_from_buffer, id);
			if(error_code == STRGMGR_SUCCESS) break;
			else if(error_code == STRGMGR_FAILURE) {
				snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "Connection to SQL server lost\n");
            	write_to_fifo(fifo_buffer);
			}
		}
		DEBUG_PRINTF("connection failed (%d / %d)", tries, DB_CONNECTION_TRIES);
		sleep(1);	
	}
	
	ERR_HANDLER(db == NULL, STRGMGR_FAILURE);
	ERR_HANDLER(error_code == STRGMGR_FAILURE, STRGMGR_FAILURE);
	disconnect(db);

	snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "Connection to SQL server lost\n");
    write_to_fifo(fifo_buffer);
	DEBUG_PRINTF("finished strgmgr\n");

	pthread_barrier_wait(&barrier);
	return MAIN_PTHREAD_SUCCESS;
}

int main(int argc, char *argv[]) {

	int port_number;

	if (argc != 2) {
	printf("Use this program with 1 command line option: \n");
  	printf("\t%-15s : TCP server port number\n", "\'server port\'");
	exit(EXIT_SUCCESS);
  	}
	else port_number = atoi(argv[1]);

	__pid_t pid = fork();
	ERR_HANDLER(pid < 0, MAIN_FORK_FAILURE);
	
	if(pid > 0) {
		DEBUG_PRINTF("parent process started pid:(%d)\n", pid);
		init_fifo_write();

		pthread_t conn_thread, data_thread, strg_thread;
		int reading_buffer_id1 = 0, reading_buffer_id2 = 1;
		int error_code;

		error_code = sbuffer_init(&sensor_buffer);
		ERR_HANDLER(error_code != SBUFFER_SUCCESS, SBUFFER_FAILURE);

		pthread_mutex_init(&mutex, NULL);
		pthread_barrier_init(&barrier, NULL, 4);

		pthread_create(&conn_thread, NULL, connmgr, &port_number);
		pthread_create(&data_thread, NULL, datamgr, &reading_buffer_id1);
		pthread_create(&strg_thread, NULL, strgmgr, &reading_buffer_id2);

		pthread_barrier_wait(&barrier);

		pthread_join(conn_thread, NULL);
		pthread_join(data_thread, NULL);
		pthread_join(strg_thread, NULL);

		pthread_mutex_destroy(&mutex);
		pthread_barrier_destroy(&barrier);

		error_code = sbuffer_free(&sensor_buffer);
		ERR_HANDLER(error_code != SBUFFER_SUCCESS, SBUFFER_FAILURE);

		fprintf(fifo_write, "%s", TO_STRING(EXIT_PIPE_CONDITION));
		fclose(fifo_write);
		DEBUG_PRINTF("parent process terminated pid:(%d)\n", pid);
	}
	else if(pid == 0) {
		DEBUG_PRINTF("child process started pid:(%d)\n", pid);
		init_fifo_read();

    	FILE *gateway = fopen(TO_STRING(GATEWAY), "w+");
		if(gateway == NULL) DEBUG_PRINTF("error creating log\n");
		ERR_HANDLER(gateway == NULL, MAIN_FILE_ERROR);

    	read_from_fifo(gateway);
    	fclose(fifo_read);
    	fclose(gateway);
		DEBUG_PRINTF("child process terminated pid:(%d)\n", pid);
		exit(EXIT_SUCCESS);
	}
	waitpid(pid, NULL, WUNTRACED);
	printf("\nprogram terminated succesfully\n");
	return 0;
}
