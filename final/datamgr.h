#ifndef DATAMGR_H_
#define DATAMGR_H_
 
#ifndef SET_MAX_TEMP
  #error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
  #error SET_MIN_TEMP not set
#endif

/*
 *  This method holds the core functionality of your datamgr. It takes in 1 file pointer from a map file and parses them. 
 *  after that it receives sensor data from a buffer and adds them in the internal pointer list and all log messages should go through a fifo pipe
 */
void datamgr_parse_sensor_buffer(FILE *fp_sensor_map, int(*read_from_buffer)(sensor_data_t*, int), int id, void(*write_to_fifo)(char*));

/*
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();
    
/*   
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);


/*
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);


/*
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);


/*
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 */
int datamgr_get_total_sensors();

#endif  //DATAMGR_H_

