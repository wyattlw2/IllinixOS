// need to support most keys 
// need to keep track of screen l ocation
// interept crtl + L or l clears screen and puts cursor on top
// need to support backspace and line-buffereed input
// buffer is 128 characters
// need an external interface that supports external data to terminal output
//      how user progrmas pass parameters toterminal
//      check for bad inputs
// terminal  read waits for user to type something in and press enter. AND the 
//  buffer should be filled with what was typed
// terminal write just writes a string to the screen according to the buffer
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

int t_open();
int t_close();
int32_t t_read(int32_t fd, void* buf, int32_t nbytes);
int32_t t_write(int32_t fd, const void* buf, int32_t nbytes);

// stores everything the person types
// need to handle buffer overflow and when under 128
// remember that one character is used to create a new line so
//      we be wary of writing over 127 characters, we just ignore those characters
unsigned char kb_buff[128];

// buf copies from kb_buff and is used to write to the screen
unsigned char buf[128];

// kb_handler needs to handle cases where we go over the width of a line and it writes on the
// next line and deleting one line can go back to the previous line

// 1) expand the kb_handler to handle more scancodes
// 2) clear the screen and be able to start typing on the top
// 3) read - kb_buff hanlding
// 4)      - copying into buf
// 5) finish writing to the terminal with buf

#endif
