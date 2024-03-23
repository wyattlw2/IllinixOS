#include "types.h"
#include "x86_desc.h"
// #include "assem_link.S"
// #include "assem_link.h"
#include "lib.h"
extern void initialize_idt();
void set_exception_params(idt_desc_t * idt_array_index, int vec);
extern void exec_handler();
extern void sys_handler();
