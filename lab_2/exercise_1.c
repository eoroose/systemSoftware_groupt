#include <stdio.h>
#include <stdlib.h>

typedef struct {
    short day, month;
    unsigned year;
} date_t;

void date_struct( int day, int month, int year, date_t *date) {
    date->day = (short)day;
    date->month = (short)month;
    date->year = (unsigned)year;   
}

int main(void) {
    int day, month, year;
    date_t d;
    printf("\nGive day, month, year:");
    scanf("%d %d %d", &day, &month, &year);
    date_struct(day, month, year, &d);
    printf("\ndate struct values: %d-%d-%d", d.day, d.month, d.year);
    return 0;
}