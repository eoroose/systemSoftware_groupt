#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>


#include "config.h"
#include "sbuffer.h"

#define FILE_ERROR(fp,error_msg) 	do { \
					  if ((fp)==NULL) { \
					    printf("%s\n",(error_msg)); \
					    exit(EXIT_FAILURE); \
					  }	\
					} while(0)

sbuffer_t *buffer;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
char flag = 0;

void *writer(void *arg) {
	FILE *file = (FILE *)arg;
	sensor_data_t data;
	
	while ( fread(&(data.id), sizeof(sensor_id_t), 1, file) > 0 &&
            fread(&(data.value), sizeof(sensor_value_t), 1, file) > 0 &&
            fread(&(data.ts), sizeof(sensor_ts_t), 1, file) > 0) { 	

				pthread_mutex_lock(&mutex);
				sbuffer_insert(buffer, &data);
				pthread_mutex_unlock(&mutex);
	}
	flag = 1;
	printf("finished writer\n");
	pthread_barrier_wait(&barrier);
} 

void *reader1(void *arg) {
	int id = *(int*)arg;
	FILE *fp_reader1;
	fp_reader1 = fopen("reader1.txt", "w+");
	sensor_data_t data;
	int message;

	while (1) {	

		pthread_mutex_lock(&mutex);
		message = sbuffer_remove2(buffer, &data, id);
		pthread_mutex_unlock(&mutex);

		if(message == SBUFFER_NO_DATA && flag == 1) break;

		fprintf(fp_reader1, "sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);

	}
	fclose(fp_reader1);
	printf("finished reader1\n");
	pthread_barrier_wait(&barrier);
}

void *reader2(void *arg) {
	int id = *(int*)arg;
	FILE *fp_reader2;
	fp_reader2 = fopen("reader2.txt", "w+");
	sensor_data_t data;
	int message;

	while (1) {	

		pthread_mutex_lock(&mutex);
		message = sbuffer_remove2(buffer, &data, id);
		pthread_mutex_unlock(&mutex);

		if(message == SBUFFER_NO_DATA && flag == 1) break;

		fprintf(fp_reader2, "sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);

	}
	fclose(fp_reader2);
	printf("finished reader2\n");
	pthread_barrier_wait(&barrier);
}


int main(int argc, char *argv[]) {

	char* sensor_data;

	if (argc != 2) {
	printf("Use this program with 1 command line option: \n");
  	printf("\t%-15s : sensor data\n", "\'binary file\'");
	exit(EXIT_SUCCESS);
  	}
	else sensor_data = argv[1];

	FILE *file;
	pthread_t p1, p2, p3;
	int id1 = 0, id2 = 1;

	file = fopen(sensor_data, "r");
  	FILE_ERROR(file, "Couldn't open sensor_data\n");
	
	pthread_mutex_init(&mutex, NULL);
	pthread_barrier_init(&barrier, NULL, 4);
	if(sbuffer_init(&buffer) != SBUFFER_SUCCESS) exit(EXIT_FAILURE);


	pthread_create(&p1, NULL, writer, file);
	pthread_create(&p2, NULL, reader1, &id1);
	pthread_create(&p3, NULL, reader2, &id2);

	pthread_barrier_wait(&barrier);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_barrier_destroy(&barrier);

	fclose(file);
	if(sbuffer_free(&buffer) != SBUFFER_SUCCESS) exit(EXIT_FAILURE);

	return 0;
}
