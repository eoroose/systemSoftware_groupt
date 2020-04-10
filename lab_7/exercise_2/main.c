#include "sensor_db.h"
#include <stdio.h>

#define sensor_data "sensor_data"
//<sensor ID><temperature><timestamp>

//static int callback(void *not_used, int argc, char **argv, char **col_name) {
//    int i;
//    for(i = 0; i < argc; i++) {
//        printf("%s = %s\n", col_name[i], argv[i] ? argv[i] : "NULL");
//    }
//    printf("\n");
//    return 0;
//}

char file_to_db(DBCONN *db) {
    FILE *data_file;

    data_file = fopen(sensor_data, "rb");

    if(data_file == NULL) {
        printf("data file not found\n");
        return 1;
    }
    
    insert_sensor_from_file(db, data_file);
    fclose(data_file);
    return 0;
}

DBCONN *db;

int main() {
    db = init_connection(1);
    if(db == NULL) return 0;

    printf("insterting data to db...\n");
    if(file_to_db(db)) return 0;
    //if(find_sensor_all(db, callback)) return 0;
    //if(find_sensor_by_value(db, 20, callback)) return 0;
    //if(find_sensor_exceed_value(db, 20, callback)) return 0;
    //if(find_sensor_by_timestamp(db, 1574863783, callback)) return 0;
    //if(find_sensor_after_timestamp(db, 1574861263, callback)) return 0;
    disconnect(db);

    return 0;
}
