


#include "types.h"
#include "x86_desc.h"

extern void initialize_idt();
void set_exception_params(idt_desc_t idt_array_index);