#include "terminal.h"

char* kb_buff;
int kb_idx = 0;
char* buf;

int t_open() {
    // initializes terminal or nothing
    return 0;
}

int t_close() {
    // clear any terminal specific variables
    return 0;
}

int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
    int i;
    for (i = 0; i < nbytes  - 1; i++) {
        ((char*)buf)[i] = kb_buff[i]; 
    }
    kb_buff[nbytes - 1] = '\n';

    return nbytes;
}

int32_t t_write(int32_t _fd, const void* buf, int32_t nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        putc(((char*)buf)[i]);    
    }
    for (i = 0; i < nbytes; i++) {
        ((char*)buf)[i] = '\0';   
    }
    kb_idx = 0;
    return 0;
}
