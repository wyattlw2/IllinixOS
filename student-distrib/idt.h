#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "asm_macro.h"
int EXCEPTION_FLAG;
int CLEAR_SCREEN_FLAG;
int SHELLPROMPT_DELETE_FLAG;
extern void initialize_idt();
void set_exception_params(idt_desc_t * idt_array_index, int vec);
extern void exec_handler();
extern void sys_handler();
extern uint16_t og_x;
extern uint16_t og_y;

extern int setup;
extern int next_row_flag;


