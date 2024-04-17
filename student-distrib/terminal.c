#include "terminal.h"
#include "file_sys_driver.h"

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
// int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
//     int i; // loop index
//     int arg_start = 0;
//     int b = 0;
//     int count = 0;
//     int j = 0;

//     while (1) {
//         count = 0;
//         for (i = 0; i < nbytes; i++) { // copy every character
//             if (kb_buff[i] == '\n') {
//                 b = 1;
//                 break;
//             }

//             // if (!arg_start) { // if we haven't gotten to the argument yet
//                 ((char*)buf)[i] = kb_buff[i];
                // if (kb_buff[i] == ' ') {
                //     // arg_start = i + 1;
                //     ((char*)buf)[i] = '\0';
                // }
//                 count++;
//             // } 
            // else { // we're at the argument, start filling buffer
            //     if(arg_start+j < 128)
            //     get_args_buf[arg_start + j] = kb_buff[i];
            //     j++;
//             }

//             if (CLEAR_SCREEN_FLAG == 1) { // we did this so that whenever we call crtl + l in shell, it is able to break out and we can see the 391OS>
//                 ((char* )buf)[0] = '\0'; // set this so that shell doesn't try and execute whatever is in the buf, it recognizes it as empty and continues
//                 b = 1;
//                 CLEAR_SCREEN_FLAG = 0; // This could cause problems later, but its fine for now
//                 break;
//             }
        
//         if (b) {
//             for (i = 0; i < nbytes; i++) { // clear every character
//                 kb_buff[i] = '\t';
//             }
//             // break;
//         }
//     }
//         return count;
//     }

int32_t t_read(int32_t fd, void* buf, int32_t nbytes) {
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
        count = 0;
        for (i = 0; i < upper_bound; i++) { // copy every character
            ((char*)buf)[i] = kb_buff[displayed_terminal][i];
            // (get_args_buf)[i] = kb_buff[i];
            count++;
            if (kb_buff[displayed_terminal][i] == '\n') {
                b = 1;
                break;
            }
             if (kb_buff[displayed_terminal][i] == ' ') { // might want to set a flag such that for the rest of this string will be null characters or something -DVT
                    // arg_start = i + 1;
                    ((char*)buf)[i] = '\0';
                }
            if (CLEAR_SCREEN_FLAG == 1) { // we did this so that whenever we call crtl + l in shell, it is able to break out and we can see the 391OS>
                ((char* )buf)[0] = '\0'; // set this so that shell doesn't try and execute whatever is in the buf, it recognizes it as empty and continues
                b = 1;
                CLEAR_SCREEN_FLAG = 0; // This could cause problems later, but its fine for now
                break;
            }
        }
        if (b) {
            
           
            for (i = 0; i < 128; i++) { // clear every character in the kb buff
                kb_buff[displayed_terminal][i] = '\t';
            }
            break;
        }
    }
    // get_args_buf[0] = 'f';
    // get_args_buf[1] = 'r';
    // get_args_buf[2] = 'a';
    // get_args_buf[3] = 'm';
    // get_args_buf[4] = 'e';
    // get_args_buf[5] = '0';
    // get_args_buf[6] = '.';
    // get_args_buf[7] = 't';
    // get_args_buf[8] = 'x';
    // get_args_buf[9] = 't';
    // get_args_buf[10] = '\0';
    // return 10;
    for(i=0; i<128; i++){
        get_args_buf[i] = ((char*)buf)[i];
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
        if(((char*)buf)[i] != NULL) {
            putc(((char*)buf)[i]);
        }
    }
    // for (i = 0; i < nbytes; i++) {
    //     ((char*)buf)[i] = '\0';
    // }

    return nbytes;
}
