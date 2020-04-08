#include <stdio.h>
#include <stdlib.h>

int main() {
    system("clear");
    system("ls -l > files.txt");
    system("sort files.txt | more");
    return 0;
}