#include "datamgr.h"

#define sensor_map "room_sensor.map"
//<room ID><space><sensor ID><\n>
#define sensor_data "sensor_data"
//<sensor ID><temperature><timestamp>

int main() {

    FILE *map_file, *data_file;
    
    map_file = fopen(sensor_map, "r");

    if(map_file == NULL) {
        printf("map file not found\n");
        return 0;
    }

    data_file = fopen(sensor_data, "rb");

    if(data_file == NULL) {
        printf("data file not found\n");
        return 0;
    }
    
    datamgr_parse_sensor_files(map_file, data_file);
    fclose(map_file);
    fclose(data_file);

    datamgr_free();

    printf("test succesful\n");

    return 0;
}
