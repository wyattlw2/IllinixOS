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
    int j;
    for (i = 0; i < MAX_BUFF_SIZE; i++) { // copy every character
        ((char*)buf)[i] = kb_buff[i];
        if (kb_buff[i] == '\n') { // kb buffer only CLEARED when enter is pressed
            for (j = 0; j < MAX_BUFF_SIZE; j++) { // clear the kb_buffer
                kb_buff[j] = '\t'; // code for not print anything
                IS_KB_CLEAR = 1;
            }
        }
    }
    return nbytes;
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

    if (IS_KB_CLEAR) { // enter is pressed so we print what is stored
        // uint16_t pos = get_cursor_position();
        // uint16_t y = pos / NUM_COLS;
        // uint16_t x = pos % NUM_COLS;
        for (i = 0; i < 128; i++) { // print every character
            putc(((char*)buf)[i]);
        }
        IS_KB_CLEAR = 0;
    }
    return 0;
}
