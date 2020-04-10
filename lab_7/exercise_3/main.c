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

    __pid_t pid = fork();

    if(pid > 0) {
        //database
        printf("logging @ gateway.log ...\n");
        db = init_connection(1);
        if(db == NULL) return 0;
        
        if(file_to_db(db)) return 0;
        disconnect(db);
        printf("parent process over\n");
    } else {
        //logging
        gateway();
        printf("child process over\n");
    }

    return 0;
}
