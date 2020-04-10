#include "sensor_db.h"
#define GATEWAY "gateway.log"
#define FIFO "/tmp/myfifo"
#define EXIT "exit"

char execute(DBCONN*, char*, callback_t);
FILE *fd;

DBCONN * init_connection(char clear_up_flag) {

    char *my_fifo = FIFO;
    mkfifo(my_fifo, 0666);
    fd = fopen(my_fifo, "w");

    if(fd == NULL) {
        printf("error opening fifo\n");
        exit(1);
    }

    DBCONN *db;
    int rc = sqlite3_open(TO_STRING(DB_NAME), &db);

    if(rc != SQLITE_OK) {
        fprintf(fd, "can't open database: %s\n", sqlite3_errmsg(db));
        fprintf(fd, EXIT);
        fclose(fd);
        return NULL;
    }
    fprintf(fd, "opened database successfully\n");
    
    if(clear_up_flag) {
        char *sql = "DROP TABLE IF EXISTS " TO_STRING(TABLE_NAME) ";";
        if(execute(db, sql, 0)) return NULL;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS " TO_STRING(TABLE_NAME) "("
                            "id              INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "sensor_id       INTEGER, "
                            "sensor_value    DECIMAL(4,2), "
                            "timestamp       TIMESTAMP       );";

    if(execute(db, sql, 0)) return NULL;
    
    fprintf(fd, "created the " TO_STRING(TABLE_NAME) " table successfully\n");
    return db;
}

void disconnect(DBCONN *conn) {
    fprintf(fd, "closed database successfully\n");
    fprintf(fd, EXIT);
    fclose(fd);
    sqlite3_close(conn);
}

int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    
    char sql[500];
    snprintf(sql, 500,  "INSERT INTO " TO_STRING(TABLE_NAME)
                        " (sensor_id, sensor_value, timestamp)"
                        " VALUES(%hu, %f, %ld);",
                        id, value, ts);

    if(execute(conn, sql, 0)) return 1;
    fprintf(fd, "inserted (id:%hu value:%f ts:%ld) successfully\n", id, value, ts);
    return 0;
}


int insert_sensor_from_file(DBCONN * conn, FILE * sensor_data) {

    sensor_data_t sensor;
    while ( fread(&(sensor.id), sizeof(sensor_id_t), 1, sensor_data) > 0 &&
            fread(&(sensor.value), sizeof(sensor_value_t), 1, sensor_data) > 0 &&
            fread(&(sensor.ts), sizeof(sensor_ts_t), 1, sensor_data) > 0) {     
        
        if(insert_sensor(conn, sensor.id, sensor.value, sensor.ts)) {
            fprintf(fd, "data insertion failed\n");
            fprintf(fd, "exit");
            fclose(fd);
            return 1;
        }
    }
    return 0;
}

int find_sensor_all(DBCONN * conn, callback_t f) {
    char *sql = "SELECT * FROM " TO_STRING(TABLE_NAME) ";";
    return execute(conn, sql, f);
}

int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f) {
    char sql[500];
    snprintf(sql, 500, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE sensor_value = %f", value);
    return execute(conn, sql, f);
}

int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f) {
    char sql[500];
    snprintf(sql, 500, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE sensor_value > %f", value);
    return execute(conn, sql, f);
}

int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f) {
    char sql[500];
    snprintf(sql, 500, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE timestamp = %ld", ts);
    return execute(conn, sql, f);
}

int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f) {
    char sql[500];
    snprintf(sql, 500, "SELECT * FROM " TO_STRING(TABLE_NAME) " WHERE timestamp > %ld", ts);
    return execute(conn, sql, f);
}

char execute(DBCONN *db, char *sql, callback_t f) {
    char *errmsg = 0;
    int rc = sqlite3_exec(db, sql, f, 0, &errmsg);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return 1;
    }
    return 0;
}

void gateway() {
    char buffer[255];
    int sequence = 1;

    FILE *fd1;
    char *my_fifo = FIFO;
    mkfifo(my_fifo, 0666);
    fd1 = fopen(my_fifo, "r");
    
    if(fd1 == NULL) {
        printf("error opening fifo\n");
        exit(1);
    }

    FILE *fp;
    fp = fopen(GATEWAY, "w+");

    while(fgets(buffer, 255, (FILE*)fd1) != NULL) {
        if(strcmp(buffer, EXIT) == 0) break;
        fprintf(fp, "%d:\ttime: %ld  message: %s", sequence, time(NULL), buffer);
        sequence++;
    }
    fclose(fd1);
    fclose(fp);
}