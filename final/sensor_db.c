#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "sensor_db.h"

#define SQL_LENGTH 500  // max amount of characters per sql command

// executes sql command, returns either STRGMGR_SUCCESS or STRGMGR_FAILURE
char execute(DBCONN*, char*, callback_t);

DBCONN * init_connection(char clear_up_flag, void(*write_to_fifo)(char*)) {
    DBCONN *db;
    char fifo_buffer[CONFIG_FIFO_SIZE];
    int rc = sqlite3_open(TO_STRING(DB_NAME), &db);

    if(rc != SQLITE_OK) {
        DEBUG_PRINTF("Unable to connect to SQL server\n");
        snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "Unable to connect to SQL server\n");
        write_to_fifo(fifo_buffer);
        return NULL;
    }
    DEBUG_PRINTF("opened database successfully\n");
    snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "Connection to SQL server established\n");
    write_to_fifo(fifo_buffer);

    if(clear_up_flag == STRGMGR_CLEAR_TABLE_TRUE) {
        char *sql = "DROP TABLE IF EXISTS " TO_STRING(TABLE_NAME) ";";
        if(execute(db, sql, 0) == STRGMGR_FAILURE) return NULL;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS " TO_STRING(TABLE_NAME) "("
                            "id              INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "sensor_id       INTEGER, "
                            "sensor_value    DECIMAL(4,2), "
                            "timestamp       TIMESTAMP       );";

    if(execute(db, sql, 0) == STRGMGR_FAILURE) return NULL;
    
    DEBUG_PRINTF("created the " TO_STRING(TABLE_NAME) " table successfully\n");
    snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "New table " TO_STRING(TABLE_NAME) " created\n");
    write_to_fifo(fifo_buffer);
    return db;
}

void disconnect(DBCONN *conn) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);
    sqlite3_close(conn);
    DEBUG_PRINTF("closed database successfully\n");
}

int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char sql[SQL_LENGTH];
    snprintf(sql, SQL_LENGTH,  "INSERT INTO " TO_STRING(TABLE_NAME)
                                " (sensor_id, sensor_value, timestamp)"
                                " VALUES(%hu, %f, %ld);",
                                id, value, ts);

    if(execute(conn, sql, 0) == STRGMGR_FAILURE) return STRGMGR_FAILURE;

    fprintf(stdout, "\tinserted (id:%hu value:%.2f) to DB\n", id, value);
    return STRGMGR_SUCCESS;
}

int insert_sensor_from_buffer(DBCONN *conn, int(*read_from_buffer)(sensor_data_t*, int), int id) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);
    ERR_HANDLER(id < 0, STRGMGR_INVALID_ERRROR);
    sensor_data_t sensor;
    int message;
    while (1) {     
        
        // seeing if there is sensor_data in the buffer, if not wait until there is,
        // if there is no data in the buffer, and flag is raised, break loop and end program
        message = read_from_buffer(&sensor, id);
        if(message == CONFIG_THREAD_WAIT) continue;
        if(message == CONFIG_THREAD_TERMINATE) break;

        if(insert_sensor(conn, sensor.id, sensor.value, sensor.ts) == STRGMGR_FAILURE) {
            DEBUG_PRINTF("data insertion failed\n");
            return STRGMGR_FAILURE;
        }
    }
    return STRGMGR_SUCCESS;
}

int find_sensor_all(DBCONN * conn, callback_t f) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char *sql = "SELECT * FROM " TO_STRING(TABLE_NAME) ";";
    return execute(conn, sql, f);
}

int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char sql[SQL_LENGTH];
    snprintf(sql, SQL_LENGTH, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE sensor_value = %f", value);
    return execute(conn, sql, f);
}

int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char sql[SQL_LENGTH];
    snprintf(sql, SQL_LENGTH, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE sensor_value > %f", value);
    return execute(conn, sql, f);
}

int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char sql[SQL_LENGTH];
    snprintf(sql, SQL_LENGTH, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE timestamp = %ld", ts);
    return execute(conn, sql, f);
}

int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f) {
    ERR_HANDLER(conn == NULL, STRGMGR_INVALID_ERRROR);

    char sql[SQL_LENGTH];
    snprintf(sql, SQL_LENGTH, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE timestamp > %ld", ts);
    return execute(conn, sql, f);
}

//private functions

char execute(DBCONN *db, char *sql, callback_t f) {
    ERR_HANDLER(db == NULL, STRGMGR_INVALID_ERRROR);

    char *errmsg = 0;
    int rc = sqlite3_exec(db, sql, f, 0, &errmsg);
    if(rc != SQLITE_OK) {
        DEBUG_PRINTF("SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return STRGMGR_FAILURE;
    }
    return STRGMGR_SUCCESS;
}