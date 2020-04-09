#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "ma_malloc.h"

//You need to make a sketch of your memory pool at different points in your program indicated by a number
//Which parts are free, which are allocated?
//Where are your headers/footers - how many bytes do they take? What is stored in it?
//Where are the pointers returned by ma_malloc?

int main(int argc, char *argv[]) {
    char* ptr[] = {NULL,NULL,NULL};

    //sketch 1
    ma_init();

    //sketch 2
    assert( NULL == ma_malloc(600) ); //->should return NULL
    assert( NULL != ma_malloc(200) ); //->should not return NULL;
    ma_init();
    ptr[0] = ma_malloc(400); //->should not return NULL (ma_init() clears all)

    //sketch 3
    assert( ptr[0] != NULL );
    ma_free(ptr[0]);

    printf("\nptr[0] = 200 bytes of memory\n");
    ptr[0] = ma_malloc(200); //-> should not return NULL
    assert( ptr[0] != NULL );
    ma_print();
    printf("\nptr[1] = 100 bytes of memory\n");
    ptr[1] = ma_malloc(100); //-> should not return NULL
    assert( ptr[1] != NULL );
    ma_print();
    printf("\nptr[2] = 100 bytes of memory\n");
    ptr[2] = ma_malloc(100); //-> should not return NULL

    //sketch 4
    assert( ptr[2] != NULL );
    ma_print();
    printf("\nfreeing ptr[1]\n");
    ma_free(ptr[1]);
    ma_print();

    //sketch 5
    printf("\nfreeing ptr[2]\n");
    ma_free(ptr[2]); //-> mind the sequence, first free 1 than 2
    ma_print();

    //sketch 6
    printf("\n250 bytes of memory\n");
    assert( NULL != ma_malloc(250) ); //-> should not return NULL (test for coalescing)
    ma_print();
    
    return 0;
}