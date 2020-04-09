#include <stdio.h>
#include <stdlib.h>

void swap_pointers(void **ptr1, void **ptr2) {
    void *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

int main(void) {
    int a = 1;
    int b = 2;
    void *p = &a;
    void *q = &b;
    printf("address of p = %p and q = %p\n", p, q);
    // prints p = &a and q = &b
    swap_pointers(&p, &q);
    printf("address of p = %p and q = %p\n", p, q);
    // prints p = &b and q = &a
    return 0;
}   