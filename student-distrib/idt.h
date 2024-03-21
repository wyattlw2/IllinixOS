


#include "types.h"
#include "x86_desc.h"
// #include "assem_link.S"
// #include "assem_link.h"
#include "lib.h"
extern void initialize_idt();
void set_exception_params(idt_desc_t * idt_array_index, int vec);
extern void exec_handler();
extern void sys_handler();


//RTC
int32_t rtc_set_frequency(int32_t frequency);
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close (int32_t fd);

