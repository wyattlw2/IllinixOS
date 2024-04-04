#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "asm_macro.h"





void halt (uint8_t status);
int32_t execute (uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, void* buf, int32_t nbytes);
int32_t open (uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn();
