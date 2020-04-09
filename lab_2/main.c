#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MIN -10
#define MAX 35
#define FREQUENCY 5

int main(void) {
    srand(time(NULL));

    while(1) {
        float random = (float)rand() / RAND_MAX;
        float temp = random*(MAX-MIN)+MIN;
        
        printf("Temperature = %1.2f @ ", temp);
        fflush(stdout);
        system("date");
        sleep(FREQUENCY);
    }
    return 0;
}