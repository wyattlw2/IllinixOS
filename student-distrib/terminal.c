#include "terminal.h"

#define     MAX_BUFF_SIZE       128
#define     NUM_COLS      80
// buffers for collecting keyboard data and to print it
char kb_buff[128];
int kb_idx = 0;
char buf[128];
// keeps track of whether enter is pressed to print buf
int IS_KB_CLEAR = 0;

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

/* int t_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file directory
 *         void* buf - character buf
 *         int32_t nbytes - number of bytes 
 * Return Value: nbytes - number of bytes read
 * Function: fills up the buf buffer with characters from kb_buff */
int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
    int i; // loop index

    int b = 0;
    int count = 0;
    while (1) {
        count = 0;
        for (i = 0; i < nbytes; i++) { // copy every character
            ((char*)buf)[i] = kb_buff[i];
            count++;
            if (kb_buff[i] == '\n') {
                b = 1;
                break;
            }
            if (CLEAR_SCREEN_FLAG == 1) { // we did this so that whenever we call crtl + l in shell, it is able to break out and we can see the 391OS>
                ((char* )buf)[0] = '\0'; // set this so that shell doesn't try and execute whatever is in the buf, it recognizes it as empty and continues
                b = 1;
                CLEAR_SCREEN_FLAG = 0; // This could cause problems later, but its fine for now
                break;
            }
        }
        if (b) {
            for (i = 0; i < nbytes; i++) { // clear every character
                kb_buff[i] = '\t';
            }
            break;
        }
    }


    return count;
}

/* int t_write(int32_t _fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file directory
 *         const void* buf - character buf
 *         int32_t nbytes - number of bytes 
 * Return Value: 0 - successful print
 *              -1 - failed print 
 * Function: prints all the characters in buf to the screen */
int32_t t_write(int32_t _fd, const void* buf, int32_t nbytes) {
    int i; // loop index
    if (buf == NULL) {
        return -1;
    }

    for (i = 0; i < nbytes; i++) { // print every character
        putc(((char*)buf)[i]);
    }

    return nbytes;
}
