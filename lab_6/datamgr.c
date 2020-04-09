#include "datamgr.h"
#include "lib/dplist.h"

typedef uint16_t room_id_t;

typedef struct {
    sensor_id_t sensor_id;
    room_id_t room_id;
    sensor_value_t running_avg, values[RUN_AVG_LENGTH];
    sensor_ts_t last_modified;
}Data;

Data *get_data_from_list(sensor_id_t sensor_id);

dplist_t *list;

Data *copy_Data(Data *data) {
    Data *new_data = (Data*)malloc(sizeof(Data));
    *new_data = *data;
    return new_data; 
}

void  **free_Data(Data **data) {
    free(*data);
    *data = NULL;
    return (void **)data;
}

int compare_Data(Data *data1, Data *data2) {
    if(data1->room_id > data2->room_id) return 1;
    if(data1->room_id < data2->room_id) return -1;
    return 0;
}

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    list = dpl_create(  (void *(*)(void *))copy_Data,
                        (void (*)(void **))free_Data,
                        (int (*)(void *, void *))compare_Data
    );

    Data data_map;
    while(!feof(fp_sensor_map)) {
        fscanf(fp_sensor_map, "%hd %hd\n", &(data_map.room_id), &(data_map.sensor_id));
        for(int i = 0; i < RUN_AVG_LENGTH; i++) data_map.values[i] = 0;
        dpl_insert_sorted(list, &data_map, true);
    }

    
    Data *data_sensor;
    sensor_id_t read_id;
    sensor_value_t read_value; 
    sensor_ts_t read_time_stamp;
    while(!feof(fp_sensor_data)) {
        
        fread(&(read_id), sizeof(sensor_id_t), 1, fp_sensor_data);
        fread(&(read_value), sizeof(sensor_value_t), 1, fp_sensor_data);
        fread(&(read_time_stamp), sizeof(sensor_ts_t), 1, fp_sensor_data);

        data_sensor = get_data_from_list(read_id);
        if(data_sensor == NULL) {
            fprintf(stderr, "sensor #%hd is not found\n", read_id);
            continue;
        }

        data_sensor->last_modified = read_time_stamp;
        data_sensor->running_avg = 0;

        for(int i = 1; i < RUN_AVG_LENGTH; i++) {
            data_sensor->running_avg += data_sensor->values[i-1] = data_sensor->values[i];
        }
        data_sensor->running_avg += data_sensor->values[RUN_AVG_LENGTH-1] = read_value;
        data_sensor->running_avg /= RUN_AVG_LENGTH;

        if(data_sensor->values[0] != 0) {
            if(data_sensor->running_avg < SET_MIN_TEMP)
                fprintf(stderr, "room %hd is too cold at %f degrees celsius\n", data_sensor->room_id, data_sensor->running_avg);
            else if(data_sensor->running_avg > SET_MAX_TEMP)
                fprintf(stderr, "room %hd is too hot at %f degrees celsius\n", data_sensor->room_id, data_sensor->running_avg);
        }
    }
}

void datamgr_free() {
    dpl_free(&list, true);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->room_id;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->running_avg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    return get_data_from_list(sensor_id)->last_modified;
}

int datamgr_get_total_sensors() {
    return dpl_size(list);
}

Data *get_data_from_list(sensor_id_t sensor_id) {
    int size = dpl_size(list);
    Data *data;
    for(int i = 0; i < size; i++) {
        data = dpl_get_element_at_index(list, i);
        if(data->sensor_id == sensor_id) return data;
    }
    ERROR_HANDLER(1, "sensor_id is invalid");
}
