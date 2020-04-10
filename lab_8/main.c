#include "connmgr.h"


int main(void) {
    int port = 5678;
    connmgr_listen(port);
    connmgr_free();
    return 0;
}