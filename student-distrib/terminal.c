#include "terminal.h"

char* kb_buff;
int kb_idx = 0;
char* buf;

/* int t_open()
 * Inputs:  none
 * Return Value: 0
 * Function: initializes terminal*/
int t_open() {
    return 0;
}

/* int t_close()
 * Inputs:  none
 * Return Value: 0
 * Function: closes terminal*/
int t_close() {
    return 0;
}

/* int t_read()
 * Inputs: int32_t fd - file directory
 *         void* buf - character buf
 *         int32_t nbytes - number of bytes 
 * Return Value: nbytes - number of bytes read
 * Function: fills up the buf buffer with characters from kb_buff */
int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
    int i; // loop index
    for (i = 0; i < nbytes  - 1; i++) { // copy every character
        ((char*)buf)[i] = kb_buff[i]; 
    }
    kb_buff[nbytes - 1] = '\n'; // add the required last new space 

    return nbytes;
}

/* int t_write()
 * Inputs: int32_t fd - file directory
 *         const void* buf - character buf
 *         int32_t nbytes - number of bytes 
 * Return Value: 0 - successful print
 *              -1 - failed print 
 * Function: prints all the characters in buf to the screen */
int32_t t_write(int32_t _fd, const void* buf, int32_t nbytes) {
    int i; // loop index
    for (i = 0; i < nbytes; i++) { // print every character
        putc(((char*)buf)[i]);    
    }
    for (i = 0; i < nbytes; i++) { // clear the buffer
        ((char*)buf)[i] = '\0';   
    }
    kb_idx = 0; // reset the kb_idx that keeps track of kb_buff
    return 0;
}
