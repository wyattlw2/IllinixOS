#include "terminal.h"

int t_open() {
    // initializes terminal or nothing
    return 0;
}

int t_close() {
    // clear any terminal specific variables
    return 0;
}

int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
    // return number of bytes read
    return nbytes;
}

int32_t t_write(int32_t _fd, const void* buf, int32_t nbytes) {
    // return 0 if successful or 1 otherwise
    return 0;
}
