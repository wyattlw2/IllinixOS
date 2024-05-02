#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

void terminal_init();
void init_pit();
void pit_handler();
void schedule();

typedef struct function_pointers_t {
    int32_t (*open)(uint8_t * filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void * buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void * buf, int32_t nbytes);
} function_pointers_t;


typedef struct fd_t {
    function_pointers_t file_operations_table_pointer;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
    //int32T (*open)(int32 fd, buf ,)
} fd_t;
// static  rtc = {rtc_open, rtc_close, };
typedef struct file_descriptor_array_t {
    fd_t fd_entry[8];
} file_descriptor_array_t;

typedef struct process_control_block_t { // THE PCB IS GOING TO BE STORED AT THE TOP OF A GIVEN KERNEL STACK
    int32_t PID;
    int32_t parent_PID; 
    uint32_t EBP;
    uint32_t ESP;
    int32_t EIP;
    int32_t terminal_idx;
    file_descriptor_array_t fdesc_array; // probably will also need some mechanism to see if stack overflow occurs
    
    //signal information
    //possibly all the registers
    //task state information
    //stack pointer for a given process
} process_control_block_t;


int32_t processes_active[6]; // need to figure out more about initializing this POSSIBLY SET IT ALL TO ZERO
process_control_block_t* PCB_array[6];  //MUST be an array of pointers right now --W
int32_t current_process_idx;

int32_t displayed_terminal;

int32_t scheduled_terminal;

typedef struct terminal_info_t {
    int32_t active_process_PID; // PID
    int32_t cursor_x;
    int32_t cursor_y;
    int16_t togx;
    int16_t togy;

    //values below must be saved for the purpose of context switches
    uint32_t EIP_SAVE;
    uint32_t CS_SAVE;
    uint32_t EFLAGS_SAVE;
    uint32_t ESP_SAVE;
    uint32_t SS_SAVE;
    uint32_t EBP_SAVE;
}terminal_info_t;

terminal_info_t terminal_processes[3];

int32_t SIGQUIT_FLAG[3];


#endif

