#include <stdio.h>
#include <stdlib.h>

typedef struct {
    short day, month;
    unsigned year;
} date_t;

void f(void) {
    int x, y, z;
    printf("%d %d %d\n", x, y, z );
}

date_t *date_struct(int day, int month, int year) {
    static date_t dummy;
    dummy.day = (short)day;
    dummy.month = (short)month;
    dummy.year = (unsigned)year;
    return &dummy;
}

int main(void) {
    int day, month, year;
    date_t *d;
    printf("\nGive day, month, year:");
    scanf("%d %d %d", &day, &month, &year);
    d = date_struct(day, month, year);
    f();
    printf("\ndate struct values: %d-%d-%d", d->day, d->month, d->year);
    return 0;
}