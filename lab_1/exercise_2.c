#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("void size: %ld\n", sizeof(void));
    printf("char size: %ld\n", sizeof(char));
    printf("short size: %ld\n", sizeof(short));
    printf("int size: %ld\n", sizeof(int));
    printf("float size: %ld\n", sizeof(float));
    printf("double size: %ld\n", sizeof(double));
    printf("long size: %ld\n\n", sizeof(long));

    printf("void pointer size: %ld\n", sizeof(void*));
    printf("char pointer size: %ld\n", sizeof(char*));
    printf("short pointer size: %ld\n", sizeof(short*));
    printf("int pointer size: %ld\n", sizeof(int*));
    printf("float pointer size: %ld\n", sizeof(float*));
    printf("double pointer size: %ld\n", sizeof(double*));
    printf("long pointer size: %ld\n\n", sizeof(long*));


    char name[10] = "eric";
    char *ptr = &name[0];
    printf("name size %ld\n", sizeof(name));
    printf("name pointer size %ld\n", sizeof(ptr));
    return 0;
}
