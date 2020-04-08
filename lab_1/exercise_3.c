#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

int main() {

    char first[MAX], second[MAX], name[MAX], str[MAX];

    printf("enter first and second name\n");
    scanf("%s %s", first, second);
    
    for(int i = 0; i < MAX; i++) {
        str[i] = toupper(second[i]);
    }
    printf("str = %s\n", str);

    if(strcmp(second, str) == 0) {
        printf("%s and %s are the same\n", second, str);
    } else {
        printf("%s and %s are the not same\n", second, str);
    }

    snprintf(name, MAX, "%s", first);
    strncat(name, " ", 2);
    strncat(name, second, 20);
    printf("full name: %s\n", name);

    char year[MAX];
    printf("enter birth year\n");
    scanf("%s", year);

    strncat(name, " ", 2);
    strncat(name, year, MAX);
    printf("%s\n", name);

    return 0;
}