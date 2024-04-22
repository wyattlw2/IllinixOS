#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "asm_macro.h"
#include "scheduling.h"
int EXCEPTION_FLAG;
int CLEAR_SCREEN_FLAG[3];
int TERMINAL1_SWITCH;
int TERMINAL2_SWITCH;
int TERMINAL3_SWITCH;
int TERMINAL_WRITE_FLAG[3]; 
extern void initialize_idt();
void set_exception_params(idt_desc_t * idt_array_index, int vec);
extern void exec_handler();
extern void sys_handler();

extern uint16_t og_x[3];    //used to keep track of original x/y positions respective to the terminal cursor
extern uint16_t og_y[3];

extern int setup[3];    //there has to be a separate setup flag for each terminal. this was originally a single element, which was causing us to accidentally delete
                        //the first shell prompt of a separate terminal by mistake
extern int next_row_flag[3];
// extern int send_eoi_kb_flag;
extern int no_parent_shell_flag;
void move_four_kb (uint8_t * src, uint8_t * dst);



