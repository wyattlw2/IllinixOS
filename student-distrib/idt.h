


#include "types.h"
#include "x86_desc.h"
// #include "assem_link.S"
// #include "assem_link.h"
#include "lib.h"
extern void initialize_idt();
void set_exception_params(idt_desc_t * idt_array_index, int vec);
extern void exec_handler();
extern void sys_handler();
uint16_t get_cursor_position(void);
void update_cursor(int x, int y);
void disable_cursor();
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
