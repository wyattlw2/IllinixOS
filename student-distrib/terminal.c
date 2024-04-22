#include "terminal.h"
#include "file_sys_driver.h"
#include "scheduling.h"

#define     MAX_BUFF_SIZE       128
#define     NUM_COLS      80
// buffers for collecting keyboard data and to print it
char kb_buff[3][128];
int kb_idx[3] = {0,0,0};
char buf[128];
char get_args_buf[128];
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
    TERMINAL_READ_FLAG[scheduled_terminal] = 1; //this also should be scheduled terminal instead of displayed terminal

    int i; // loop index
    // int j;
    int b = 0;
    int count = 0;
    int upper_bound = 0;
    if(nbytes > 128){
        upper_bound = 128;
    }else{
        upper_bound = nbytes;
    }
    while (1) {
        while(displayed_terminal != scheduled_terminal) {       //i have no idea why this shit works for scheduling but Stephen said it was cool so lets fuckin go
            continue;                                           //before this line, there was a 1/3 chance during "hello" that the entered command would be read
        }
        count = 0;
        for (i = 0; i < upper_bound; i++) { // copy every character
            ((char*)buf)[i] = kb_buff[scheduled_terminal][i];
            // (get_args_buf)[i] = kb_buff[i];
            count++;
            if (kb_buff[scheduled_terminal][i] == '\n') {
                b = 1;
                break;
            }
             if (kb_buff[scheduled_terminal][i] == ' ') { // might want to set a flag such that for the rest of this string will be null characters or something -DVT
                    // arg_start = i + 1;
                    ((char*)buf)[i] = '\0';
                }
            if (CLEAR_SCREEN_FLAG[scheduled_terminal] == 1) { // we did this so that whenever we call crtl + l in shell, it is able to break out and we can see the 391OS>
                ((char* )buf)[0] = '\0'; // set this so that shell doesn't try and execute whatever is in the buf, it recognizes it as empty and continues
                b = 1;
                CLEAR_SCREEN_FLAG[scheduled_terminal] = 0; // This could cause problems later, but its fine for now
                break;
            }
        }
        if (b) {
            
           
            for (i = 0; i < 128; i++) { // clear every character in the kb buff
                kb_buff[scheduled_terminal][i] = '\t';
            }
            break;
        }
    }
    for(i=0; i<128; i++){
        get_args_buf[i] = ((char*)buf)[i];
    }

    TERMINAL_READ_FLAG[scheduled_terminal] = 0;
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
    // TERMINAL_READ_FLAG[scheduled_terminal] = 0;
    cli();
    int i; // loop index
    if (buf == NULL) {
        return -1;
    }

    for (i = 0; i < nbytes; i++) { // print every character
        if(((char*)buf)[i] != NULL) {
            putc(((char*)buf)[i]);
        }
    }
    sti();
    return nbytes;
}
