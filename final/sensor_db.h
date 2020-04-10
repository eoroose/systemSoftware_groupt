#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <sqlite3.h>

#define DBCONN sqlite3 

#define STRGMGR_INVALID_ERRROR  1
#define STRGMGR_SUCCESS         0
#define STRGMGR_FAILURE        -1

#define STRGMGR_CLEAR_TABLE_TRUE  1   // drop exisitng table if (clear_up_flag == CLEAR_TABLE)
#define STRGMGR_CLEAR_TABLE_FALSE 0

typedef int (*callback_t)(void *, int, char **, char **);

/*
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * If the table existed, clear up the existing data if clear_up_flag is set to 1
 * Return the connection for success, NULL if an error occurs
 */
DBCONN * init_connection(char clear_up_flag, void(*write_to_fifo)(char*));

/*
 * Disconnect from the database server
 */
void disconnect(DBCONN *conn);

/*
 * Write an INSERT query to insert a single sensor measurement
 * Return zero for success, and non-zero if an error occurs
 */
int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/*
 * Write an INSERT query to insert all sensor measurements available from the buffer
 * Return zero for success, and non-zero if an error occurs
 */
int insert_sensor_from_buffer(DBCONN *conn, int(*read_from_buffer)(sensor_data_t*, int), int id);

/*
  * Write a SELECT query to select all sensor measurements in the table 
  * The callback function is applied to every row in the result
  * Return zero for success, and non-zero if an error occurs
  */
int find_sensor_all(DBCONN * conn, callback_t f);

/*
 * Write a SELECT query to return all sensor measurements having a temperature of 'value'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f);

/*
 * Write a SELECT query to return all sensor measurements of which the temperature exceeds 'value'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f);

/*
 * Write a SELECT query to return all sensor measurements having a timestamp 'ts'
 * The callback function is applied to every row in the result
 * Return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f);

/*
 * Write a SELECT query to return all sensor measurements recorded after timestamp 'ts'
 * The callback function is applied to every row in the result
 * return zero for success, and non-zero if an error occurs
 */
int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f);

#endif /* _SENSOR_DB_H_ */

