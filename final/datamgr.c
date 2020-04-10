#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "datamgr.h"
#include "lib/dplist.h"

#define DATAMGR_INVALID_ERROR 0
#define DATAMGR_INVALID_FILE  1
#define DATAMGR_MEMORY_ERROR  2

typedef uint16_t room_id_t;

// node that will go into the dplist
typedef struct {
    sensor_data_t sensor_data;
    room_id_t room_id;
    sensor_value_t running_avg, values[RUN_AVG_LENGTH];
}Data;

// callback functions for datamgr_list
Data *copy_Data(Data *data);
void  **free_Data(Data **data);
int compare_Data(Data *data1, Data *data2);

// returns the instance an instace of data from the dplist using "sensor_id"
Data *get_data_from_list(sensor_id_t sensor_id); 

// calculates running avg of "data_node" with the new addition of "buffer_sensor.value" added to the values array 
// error if both ids are not equal
void calc_running_avg(Data *data_node, sensor_data_t buffer_sensor);

//adds nodes to dplist according to the provided ".map" file
void init_list_from_map(FILE *fp_sensor_map);

dplist_t *datamgr_list;

void datamgr_parse_sensor_buffer(FILE *fp_sensor_map, int(*read_from_buffer)(sensor_data_t*, int), int id, void(*write_to_fifo)(char*)) {
    ERR_HANDLER(fp_sensor_map == NULL, DATAMGR_INVALID_ERROR);
    ERR_HANDLER(id < 0, DATAMGR_INVALID_ERROR);

    init_list_from_map(fp_sensor_map);
    
    Data *data_node;
    sensor_data_t buffer_sensor;
    int message;
    char fifo_buffer[CONFIG_FIFO_SIZE];
    while(1) {
        
        // seeing if there is sensor_data in the buffer, if not wait until there is,
        // if there is no data in the buffer, and flag is raised, break loop and end program
        message = read_from_buffer(&buffer_sensor, id);
        if(message == CONFIG_THREAD_WAIT) continue;
        if(message == CONFIG_THREAD_TERMINATE) break;

        // check if the received buffer sensor_data id is in the dplist, if not, invalid sensor node
        data_node = get_data_from_list(buffer_sensor.id);
        if(data_node == NULL) {
            DEBUG_PRINTF("received sensor data with invalid sensor node id:%d\n", buffer_sensor.id);
            snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "received sensor data with invalid sensor node id:%d\n", buffer_sensor.id);
            write_to_fifo(fifo_buffer);
            continue;
        }

        //update timestamp, update running_avg
        data_node->sensor_data.ts = buffer_sensor.ts;
        calc_running_avg(data_node, buffer_sensor);


        // write to fifo pipe if its too hot or too cold
        if(data_node->running_avg < SET_MIN_TEMP && data_node->values[0] != 0) {
            
            snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "The sensor node with id:%d reports its too cold (running avg temperature = %f)\n", data_node->sensor_data.id, data_node->running_avg);
            write_to_fifo(fifo_buffer);
        }
        else if(data_node->running_avg > SET_MAX_TEMP && data_node->values[0] != 0) {
            
            snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "The sensor node with id:%d reports its too hot (running avg temperature = %f)\n", data_node->sensor_data.id, data_node->running_avg);
            write_to_fifo(fifo_buffer);
        }
        
    }
}

void datamgr_free() {
    dpl_free(&datamgr_list, true);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->room_id;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->running_avg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->sensor_data.ts;
}

int datamgr_get_total_sensors() {
    return dpl_size(datamgr_list);
}

//private functions

Data *get_data_from_list(sensor_id_t sensor_id) {
    ERR_HANDLER(datamgr_list == NULL, DATAMGR_INVALID_ERROR);
    int size = dpl_size(datamgr_list);
    Data *data;

    for(int i = 0; i < size; i++) {
        data = dpl_get_element_at_index(datamgr_list, i);
        if(data->sensor_data.id == sensor_id) return data;
    }
    return NULL;
}

void calc_running_avg(Data *data_node, sensor_data_t buffer_sensor) {

    ERR_HANDLER(data_node->sensor_data.id != buffer_sensor.id, DATAMGR_INVALID_ERROR);

    data_node->running_avg = 0;
    for(int i = 1; i < RUN_AVG_LENGTH; i++) {
        data_node->running_avg += data_node->values[i-1] = data_node->values[i];
    }
    data_node->running_avg += data_node->values[RUN_AVG_LENGTH-1] = buffer_sensor.value;
    data_node->running_avg /= RUN_AVG_LENGTH;
}

void init_list_from_map(FILE *fp_sensor_map) {
    ERR_HANDLER(fp_sensor_map == NULL, DATAMGR_INVALID_FILE);
    datamgr_list = dpl_create(  (void *(*)(void *))copy_Data, (void (*)(void **))free_Data, (int (*)(void *, void *))compare_Data);

    Data data_map;
    while(!feof(fp_sensor_map)) {
        fscanf(fp_sensor_map, "%hd %hd\n", &(data_map.room_id), &(data_map.sensor_data.id));
        for(int i = 0; i < RUN_AVG_LENGTH; i++) data_map.values[i] = 0;
        dpl_insert_sorted(datamgr_list, &data_map, true); 
    }
}

//callback functions

Data *copy_Data(Data *data) {
    ERR_HANDLER(data == NULL, DATAMGR_INVALID_ERROR);
    Data *new_data = (Data*)malloc(sizeof(Data));
    ERR_HANDLER(new_data == NULL, DATAMGR_MEMORY_ERROR);
    *new_data = *data;
    return new_data; 
}

void  **free_Data(Data **data) {
    ERR_HANDLER(*data == NULL, DATAMGR_INVALID_ERROR);
    free(*data);
    *data = NULL;
    return (void **)data;
}

int compare_Data(Data *data1, Data *data2) {
    ERR_HANDLER(data1 == NULL, DATAMGR_INVALID_ERROR);
    ERR_HANDLER(data2 == NULL, DATAMGR_INVALID_ERROR);
    if(data1->room_id > data2->room_id) return 1;
    if(data1->room_id < data2->room_id) return -1;
    return 0;
}