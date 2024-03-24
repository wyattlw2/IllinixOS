#include "types.h"
#include "x86_desc.h"
#include "lib.h"
extern void init_keyboard();
extern void keyboard_handler();

// functions for enabling, updating cursor on the screen
uint16_t get_cursor_position(void);
void update_cursor(int x, int y);
void disable_cursor();
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
