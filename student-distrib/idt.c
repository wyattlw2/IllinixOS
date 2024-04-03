#include "idt.h"
#include "file_sys_driver.h"
#include "paging.h"
#include "x86_desc.h"   //need to include so we can modify ESP0 field of TSS -- wyatt
#include "terminal.h"
#include "rtc.h"

#define     VIDEO               0xB8000
#define     KEYBOARD_PORT       0x60       //WYATT ADDED
#define     NUM_COLS            80
#define     NUM_ROWS            25
#define     MAX_BUFF_SIZE       128
#define     SPEC_CHAR_OFFSET    54


#define KERNEL_CS   0x0010
#define KERNEL_DS   0x0018
#define USER_CS     0x0023
#define USER_DS     0x002B
#define KERNEL_TSS  0x0030
#define KERNEL_LDT  0x0038

#define FD_START                2
#define FD_END                  8
#define REGULAR_FILE                    2
#define DIRECTORY_FILE                  1
#define RTC_FILE                        0

static int32_t *rtc_functions[4] = {(int32_t*)rtc_open, (int32_t*)rtc_close, (int32_t*)rtc_read, (int32_t*)rtc_write};
static int32_t *directory_functions[4] = {(int32_t*)directory_open, (int32_t*)directory_close, (int32_t*)directory_read, (int32_t*)directory_write};
static int32_t *file_functions[4] = {(int32_t*)file_open, (int32_t*)file_close, (int32_t*)file_read, (int32_t*)file_write};
//#include "assem_link.S"
// ALL OF THIS IS FOR REFERENCE
// typedef union idt_desc_t { // MOST OF THIS WILL BE THE SAME, NAKE NOTE OF WHEN TO USE TRAP GATE VS INTERRUPT vs SYSCALL vs EXCEPTION -- DVT
//     uint32_t val[2];
//     struct {
//         uint16_t offset_15_00;
//         uint16_t seg_selector;
//         uint8_t  reserved4;
//         uint32_t reserved3 : 1; //reserved trap vs interrupt gate, find this on the internet
//         uint32_t reserved2 : 1;
//         uint32_t reserved1 : 1;
//         uint32_t size      : 1;
//         uint32_t reserved0 : 1;
//         uint32_t dpl       : 2; // privledge -- system call vs user call, etc x80 is the value that we care about int x80
//         uint32_t present   : 1;
//         uint16_t offset_31_16;
//     } __attribute__ ((packed));
// } idt_desc_t;
// ^^^^ THIS IS JUST FOR REFERENCE


// extern void de(void);
// extern void db(void);
// extern void nmi(void);
// extern void bp(void);
// extern void of(void);
// extern void br(void);
// extern void ud(void);
// extern void nm(void);
// extern void df(void);
// extern void cso(void);
// extern void ts(void);
// extern void np(void);
// extern void ss(void);
// extern void gp(void);
// extern void pf(void);
// extern void mf(void);
// extern void ac(void);
// extern void mc(void);
// extern void xf(void);
// extern void sys_call(void);



//20 exceptions on osdev
//rtc, keyboard , PIT (for scheduling comes later)

//uint32_t de, db, nmi, bp, of, br, ud, nm, df, cso, ts, np, ss, gp, pf, mf, ac, mc, xf, sys_call;

// handler for exceptions -- James
// void assembly_link_exec(int vector){
//     //asm()
//         asm(pushal) ;
//         asm(pushfl) ;
//         asm("pushl %0" :: "g" (vector)); 
//     asm("call *%0" :: "g" (exec_handler)); 
//     asm("addl $4, %%esp" ::); 
//         asm(popfl) ;
//         asm(popal) ;
//         asm(iret) ;

// }
// need to ask TA:
// need help with dealing '\' cannot print it
// need to confirm the while(1) {read write} test


//TABLE for the keyboard handler
const char table_kb[] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
, '0', '-', '=', '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
'p', '[', ']', '\0', '\0', 'a', 's', 'd', 'f', 'g', 'h' , 'j', 'k' ,'l', ';'
, '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/','\0', '\0',
'!', '@', '#', '$', '%', '^', '&', '*', '(' 
, ')', '_', '+', '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O',
'P', '{', '}', '\0', '\0', 'A', 'S', 'D', 'F', 'G', 'H' , 'J', 'K' ,'L', ':'
, '"', '~', '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'};


/*
kb_handler

Description: Handles keyboard interrupts.
    For now, this function will only take in the scancode and print the corresponding character to the screen.
Inputs: None
Outputs: None
Side effects: Handles the exception/interrupt raised by the keyboard. Upon the program receiving an exception/interrupt,
    it will jump to the keyboard handler to deal with the exception/interrupt
*/

// variables that keep track of whether shift, cap, or control is pressed
int shift = 0;
int cap = 0;
int ctrl = 0;
// variables that keep track of the x and y position of the cursor
uint16_t x;
uint16_t y;

void kb_handler() {

    unsigned char key = inb(KEYBOARD_PORT);

    // if tab is pressed
    if (key == 0x0F) {
        int i;
        for (i = 0; i < 4; i++) { // i < 4 because tab prints 4 spaces
            if (kb_idx != MAX_BUFF_SIZE - 1) {
                putc(' ');
                kb_buff[kb_idx] = ' ';
                kb_idx++;
            }
        }
        send_eoi(1);
        return;
    }
    // if space is pressed
    if (key == 0x39) {
        if (kb_idx != MAX_BUFF_SIZE - 1) { // if buffer isn't full
            kb_buff[kb_idx] = ' ';
            kb_idx++;
            putc(' ');
        }
    }

    // if backspace is pressed
    if (key == 0x0E) {
        uint16_t pos = get_cursor_position();
        x = pos % NUM_COLS;
        y = pos / NUM_COLS;
        if (x == 0 && y == 0) { // very first row
            send_eoi(1);
            return;
        } else if (x == 0 && y != 0) { // any other row
            update_xy(NUM_COLS - 1, y-1);
            putc(' ');
            if (y-1 >= user_y) { // anything below user_y space we can delete
                update_xy(NUM_COLS - 1, y-1);
                update_cursor(NUM_COLS - 1, y-1);
            }
        } else { // just deleting charcter in a row that doesn't go to other rows
            update_xy(x-1, y);
            putc(' ');
            update_xy(x-1, y);
            update_cursor(x-1, y);
        }
        if (kb_idx != 0) { // if buffer isn't empty already
            kb_idx -= 2;
            kb_buff[kb_idx] = '\t'; // code for not print anything
        }
        send_eoi(1);
        return;
    }

    // if enter is pressed
    if (key == 0x1C) {
        uint16_t pos = get_cursor_position();
        x = pos % NUM_COLS;
        y = pos / NUM_COLS;
        if (y != 0) {
            putc('\n'); // prepare a new line to print buf
        }
        if (y == 0 && x != 0) {
            putc('\n');
        }
        kb_buff[kb_idx] = '\n';
        // user_y += 2; // add 2 because we need to print the buffer value but also move to a new line

        kb_idx = 0;

        send_eoi(1);
        return;
    }

    // if LEFT or RIGHT ctrl pressed
    if (key == 0x1D) {
        ctrl = 1;
        send_eoi(1);
        return;
    // if LEFT or RIGHT ctrl released
    } else if (key == 0x9D) {
        ctrl = 0;
        send_eoi(1);
        return;
    }

    // if right or left shift is pressed
    if (key == 0x36 || key == 0x2A) {
        shift = 1;
        send_eoi(1);
        return;
    // right or left shift is released
    } else if (key == 0xAA || key == 0xB6) {  
        shift = 0;
        send_eoi(1);
        return;
    }
    // pressing caps lock
    if (key == 0x3A) {
        cap = !cap;
        send_eoi(1);
        return;
    }

    // clear screen operation
    if (ctrl && key == 0x26) {
        // reset everything to top left of screen
        clear();
        update_xy(0, 0);
        update_cursor(0, 0);
        // user_y = 0;
        send_eoi(1);
        return;
    }

    // caps open and pressing shift
    if (cap && shift) {
        if (key <= 0x37) { // if it's within our non special character bound
            char p = table_kb[key];
            // check that it's a printable character
            if (p != '\0') {
                // check if it's a number/symbol   
                if (p == '1' || p == '2' || p == '3' || p == '4' || p == '5' || p == '6' || p == '7'
                 || p == '8' || p == '9' || p == '0' || p == '-' || p == '=' || p == '[' || p == ']' || p == '\\'
                 || p == ';' || p == '\'' || p == ',' || p == '.' || p == '/') {
                    p = table_kb[key + SPEC_CHAR_OFFSET];
                }

                if (kb_idx != MAX_BUFF_SIZE - 1) { // if buffer isn't full
                    kb_buff[kb_idx] = p;
                    kb_idx++;
                    putc(p);
                }
            }
        }   
    // words all capped, symbols are normal
    } else if (cap) {
        if (key <= 0x37) { // if it's within our non special character bound
            char p = table_kb[key];
            // check that it's a printable character
            if (p != '\0') {
                // check if it's a number/symbol   
                if (!(p == '1' || p == '2' || p == '3' || p == '4' || p == '5' || p == '6' || p == '7'
                 || p == '8' || p == '9' || p == '0' || p == '-' || p == '=' || p == '[' || p == ']' || p == '\\'
                 || p == ';' || p == '\'' || p == ',' || p == '.' || p == '/')) {
                    p = table_kb[key + SPEC_CHAR_OFFSET];
                }

                if (kb_idx != MAX_BUFF_SIZE - 1) { // if buffer isn't full
                    kb_buff[kb_idx] = p;
                    kb_idx++;
                    putc(p);
                }
            }
        }       
    // words all capped, symbols are diff
    } else if (shift) {
        if (key <= 0x37) { // if it's within our non special character bound
            char p = table_kb[key + SPEC_CHAR_OFFSET];
            if (p != '\0') { // check it's printable character 
                if (kb_idx != MAX_BUFF_SIZE - 1) { // if buffer isn't full
                    kb_buff[kb_idx] = p;
                    kb_idx++;
                    putc(p);
                }
            }
        }
    // normal behav
    } else {
        if (key <= 0x37) { // if it's within our non special character bound
            char p = table_kb[key];
            if (p != '\0') { // check it's printable character
                if (kb_idx != MAX_BUFF_SIZE - 1) { // if buffer isn't full
                    kb_buff[kb_idx] = p;
                    kb_idx++;
                    putc(p);
                }
            }
        }
    }
    send_eoi(1);
}
//FFI FOR ALL HANDLERS BELOW
/*
Function Handlers

Description: Each of the handlers below are called whenever an exception arises.
    Every handler corresponds to a specific exception. Some data will be printed to the
    screen regarding the nature of the current exception.
Inputs: None
Outputs: None
Side effects: Handles the exception/interrupt raised by the system. Upon the program receiving an exception/interrupt,
    it will jump to a specific handler to deal with the exception/interrupt

*/
void exec_handler0() {
    //asm volatile("pushal") ;
    //asm volatile("pushfl");
    printf("\n EXCEPTION 0: A DIVIDE BY ZERO EXCEPTION HAS OCCURED \n");
    //asm volatile("popfl") ;
    //asm volatile("popal") ;
    //asm volatile("iret") ;
    
    while(1){}
    // asm volatile ("iret \n\  ");
}
void exec_handler1() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 1: A DEBUG EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler2() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 2: A NON-MASKABLE INTERRUPT EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler3() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 3: A BREAKPOINT EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler4() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 4: AN OVERFLOW EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler5() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 5: A BOUND RANGE EXCEEDED EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler6() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 6: AN INVALID OPCODE EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler7() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 7: A DEVICE NOT AVAILABLE EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler8() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 8: A DOUBLE FAULT EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler9() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 9: A SUSPICIOUS ERROR HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler10() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 10:INVALID TSS EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler11() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 11: SEGMENT NOT PRESENT EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler12() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 12: Stack Segment EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler13() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 13: General Protection Fault EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler14() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 14: Page Fault EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler15() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 15: Reserved EXCEPTION HAS OCCURED \n");
    while(1){}
}
void exec_handler16() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 16: x87 Floating point EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler17() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 17: Alignment Check EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler18() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 18: Machine Check EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}
void exec_handler19() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("\n EXCEPTION 19: SIMD Floating Point EXCEPTION HAS OCCURED \n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}

// handler for sys_calls -- James
// void sys_handler() {
//     //Is it recommended that we pass the arguments with registers
//     // are we going to pass the arguments in user space or kernel space, how does it work, who calls who... etc
//     //Is the assembly linkage given for us in this case --ece391 assem link


//     //There is a switch statement here for all 10 syscalls for the file system
//     //eax is going to have number -- Probably actually going to be a jumptable though



//     printf("THIS IS A SYSCALL HANDLE MOMENT");
//     // asm("popfl") ;
//     // asm("popal") ;
//     //asm("iret") ;
//     //while(1){}
//     return;
// }











// Below are a sequence of system call handlers that correspond to a subset
// of Linux's system calls. These are necessary for user-level programs like
// execute, sigtest, shell, and fish to work. Each of those programs will be calling
// some of these handlers via interrupt 0x80    -- Wyatt


int32_t num_active_processes = 0;   // cannot be decremented below 1 once it reaches that value. used in sys_halt() to determine
                                    // if halting the process will result in 0 running programs.

int32_t prev_PID = 70;  //initialized to 70 because we need to signify that the first process has no valid parent PID

#define     EIGHT_MB                            (1 << 23)// change back to 23// 4096 bytes * 8 bits per byte
#define     EIGHT_KB                            (1 << 13)
#define     PID_OFFSET_TO_GET_PHYSICAL_ADDRESS      2


void sys_halt() {
    //this function segfaults right now lol
    // printf("SYSCALL *HALT* CALLED \n\n");
    //The key to this function is using the global variable "prev_PID" to determine which process is the parent process...
    //Once you have accessed the parent PCB via PCB_array[prev_PID], just reverse what was done in sys_execute().
    //TLDR put the parent process context on the stack and set ESP0 of TSS to point to the old process's kernel stack. --W

    //idk if the prev_PID thing will work for cp 5, but we should keep it for now to get a base

    //W-- it will not. i asked a TA last night and the way cp5 works is you will have another global array for
    //each terminal. there can be three terminals. each value in the terminal array will be the value of the parent_PID
    //for the process. i can explain better in person

    //W-- yes? i think. in GDB i was returning to the handler for SYS_HALT which is wrong - it should be SYS_EXECUTE, this 
    //was also on my local code tho and i had made some additional changes

    //DVT -- hmm, so if we call return you thought it would go to exec, or it is supposed to at least

    //W-- right. the slides that Sanjeevi posted for discussion suggest that we return to the old context
    //from the parent process, which in the case of returning to shell will be the context from the last execute
    //look at the slides Sanjeevi posted 

    //DVT -- yeah maybe we need to alter the stack or something before we call return,

    //W-- I will lose internet access for the next 25 minutes or so, gotta head off the liveshare rn

    //yeah fair, ok sg // that if statement was causing one of the page faults, but hypothetically, it should just return out successfully?
    // printf("\n Made it to line 549 in halt \n");
    //i am in
    //it was pagefaulting before that too
    //the issue might be with return address from stack? or TSS esp0
    // gotcha, keep that comment there, i'm on queue but nobody is here yet :(, I'm gonna see if the if statement executes rq
    //also IDK if we are supposed to have shell be the first process, in the test case you guys are manually putting testprint in there

    //CHECKING IF WE ARE KILLING THE SHELL
    if(num_active_processes == 1)   {
        printf("\n Can't Close Shell!! \n");
        int8_t var[32] = {"shell"};
        //restarting shell sequence
            
        prev_PID = 70;                  //signify that the process has no parent process

        num_active_processes--;         //must do this here because we increment this value in execute

        processes_active[current_process_idx] = 0;  //signify that the currently executing program needs to be re-executed in the same spot!
                                                    //this is hardcoded to call shell.exe for now, but why would you want to call anything else?
        asm volatile (
            "movl %0, %%ebx;"   // Move the address of var into register ebx
            :                   // Output operand list is empty
            : "r" (var)         // Input operand list, specifying that var is an input
        );
        asm volatile (
            "movl $2, %eax"     // Set syscall number to 2 (sys_exec)
        );
        asm volatile (
            "int $0x80"         // Execute syscall
        );
        return; //cannot halt the program if there will be zero running programs
    }

    processes_active[current_process_idx] = 0;  //signify that the currently executing program needs to be re-executed in the same spot!

    page_directory[32].page_4mb.page_base_addr = PCB_array[current_process_idx]->parent_PID + PID_OFFSET_TO_GET_PHYSICAL_ADDRESS; //resetting the PID to be what it needs to be
    
    asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
    asm volatile("movl %ebx, %cr3");
    
    tss.esp0 = (EIGHT_MB - (PCB_array[current_process_idx]->parent_PID)*EIGHT_KB) - 4; // Does this need to point to the start of the stack or the actual stack pointer itself

     //updating current process index to be the parent's PID
    prev_PID = PCB_array[current_process_idx]->parent_PID;  //update previous process index to be the parent's parent_PID
    num_active_processes--;

    //Code above HAS to be before we retrieve the parent ESP because prev_PID needs to be changed to reflect the old PID

    uint8_t status;
    asm volatile("\t movb %%bl,%0" : "=r"(status)); // This line basically takes a value in a register and puts it into the variable -- 

    // printf("\n Made it to line 620 in halt \n");

    //potentially is freaking out due to this line right here:
    // printf("\n This is the value of prev_PID before it is called inside halt: %d", current_process_idx);
    int32_t treg = PCB_array[current_process_idx]->EBP;    //old value of ebp that we saved during sys_execute()
    current_process_idx = PCB_array[current_process_idx]->parent_PID;
    printf("\n EBP we are restoring INSIDE HALT IS: %d", treg);

    asm volatile ("movl %0, %%ebp;" : : "r" (treg));
    asm volatile ("movl %ebp, %esp");
    asm volatile ("pop %ebp");
    asm volatile("ret");
    
    // essentially calling ret here does not return to the asm link of sys_exec
    
    //Flush the TLB is not a bad idea

    //setup the tss again

    //undo the paging

    //needs to return status after this
    //USE A GOTO to get back to exectur -- asm jmp
    // asm volatile("jmp execute_to_halt");
}
#define     USER_PROG_0                         0x02
#define     USER_PROG_1                         0x03
#define     USER_PROG_2                         0x04
#define     USER_PROG_3                         0x05
#define     USER_PROG_4                         0x06
#define     USER_PROG_5                         0x07
#define     MAX_NUM_PROCESSES                   6
#define     VIRTUAL_USER_ADDR_WITH_OFFSET       0x08048000
#define     PAGE_DIR_INDEX_FOR_USER_PROG        32

#define     MAGIC_NUMBER_BYTE0                  0x7F
#define     MAGIC_NUMBER_BYTE1                  0x45
#define     MAGIC_NUMBER_BYTE2                  0x4C
#define     MAGIC_NUMBER_BYTE3                  0x46

// #define     EIGHT_MB                            0x00800000 // 4096 bytes * 8 bits per byte
// #define     EIGHT_KB                            0x00002000





//TA (Jeremy I think) specified that for CP3, we do not need to worry about having multiple shells running at once - a shell can open a
//shell, but for now we do not need to worry about more than one program executing at a time. --W
int32_t sys_execute() {
    register uint32_t ebp asm("ebp");
    register uint32_t esp asm("esp");
    uint32_t ebp_save = ebp;
    uint32_t esp_save = esp;

    int32_t retval = 256;      // sys_execute needs to return 256 in the case of an exception
    uint8_t * command;
    asm volatile("\t movl %%ebx,%0" : "=r"(command)); // This line basically takes a value in a register and puts it into the variable
    // register uint32_t ebx asm("ebx");
    //command = ()
    //might need to add some checks to see if the file is valid other than
    //Rden by name   
    dentry_struct_t exec_dentry;  
    int32_t found_file = read_dentry_by_name(command, &exec_dentry);
    // uint8_t buffer[60000];
    
    
    if(found_file == -1){
        printf("\n The inputted file was invalid. \n");
        return -1; // might need more checks for this lmao
    }
    int i;

    //process_activating is going to be the process ID NUMBER
    // process_control_block_t * PCB;  //going to turn into a global variable for now -- W

    process_control_block_t * PCB;
    int PID = 500; // ridiculous value, if it is still 500, we didn't find a process and we return out
        for(i = 0; i< MAX_NUM_PROCESSES; i++){ // start at process 
            if(processes_active[i] == 0){ // this process is empty and thus we assign the virtual addr
                num_active_processes++;
                processes_active[i] = 1;
                PID = i; // ASSIGNING PROCESS ID NUMBER
                PCB = (process_control_block_t *) (EIGHT_MB - (PID+1)*EIGHT_KB); // ASSIGNS THE ADDRESS OF THE PCB based on what process it is
                PCB_array[i] = PCB;
                PCB->parent_PID = prev_PID; // specifies if the current process is a child of another process.
                                            // if it is, set it equal to the PID of the parent process.
                                            // otherwise, set the PID to an absolutely crazy value to signify that it
                                            // is not a child process    
                // printf("(Inside Exec) Prev PID: %d\n\n", prev_PID);

                switch(PID){
                    case(0):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_0;
                        break;
                    case(1):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_1;
                        break;
                    case(2):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_2;
                        break;
                    case(3):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_3;
                        break;
                    case(4):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_4;
                        break;
                    case(5):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_5;
                        break;
                }
                
                //FLUSH TLB
                asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
                asm volatile("movl %ebx, %cr3");

                break;
            } // HOW does this not break if we had multiple 

        }
        if(PID == 500){
            printf("\n The Maximum Number of Processes are being used \n");
            return -1; // MIGHT NEED MORE THAN A RETURN HERE, WORRY ABOUT IT LATER
        }

        //TIME TO LOAD THE USER LEVEL PROGRAM
        
        //Before loading file data into virtual address space, we need to check the first four bytes of the file
        //The first four bytes must correspond to ELF
        uint8_t mag_num_buf[30]; // header is actually 40 bytes
        int32_t mag_num_check = read_data((&exec_dentry)->inode_number, 0, mag_num_buf , 30);
        
        // int8_t eip_ll = mag_num_buf[27]; // should give our new eip value -- since it is little endian I made it reverse order, could be off
        // int8_t eip_l = mag_num_buf[26];
        // int8_t eip_r = mag_num_buf[25];
        // int8_t eip_rr = mag_num_buf[24];
        int32_t EIP_save = ((uint32_t*)mag_num_buf)[6]; // Extracting the EIP from the string
        // printf("\n EIP TRY 2: %d", EIP_try2);
        
        // int32_t EIP_save = eip_ll << 24 | eip_l << 16 | eip_r << 8 | eip_rr;
        if(mag_num_check == -1){
            printf(" \n Something screwed up inside the excecutable file, you should really check that out \n");
            return -1;
        }
        if((mag_num_buf[0] != MAGIC_NUMBER_BYTE0 )||( mag_num_buf[1] != MAGIC_NUMBER_BYTE1 )||( mag_num_buf[2] != MAGIC_NUMBER_BYTE2 )||( mag_num_buf[3] != MAGIC_NUMBER_BYTE3)){
            printf(" \n Something fucked up inside the excecutable file, you should really check that shit out \n");
            return -1;
        }


        uint8_t * user_start = (uint8_t *)VIRTUAL_USER_ADDR_WITH_OFFSET;
        int32_t status = read_data((&exec_dentry)->inode_number, 0, user_start, 0x400000);
        if(status == -1){
            printf("\n Something went wrong when copying the data over \n");
        }
                                                                                                // ONE GLOBAL TSS
                                                                                                //TSS Has information about how to get back to kernel, ie HALT?
                                                                                                //when we hit up checkpoint 5 is this messed up because 
        
        //int32_t ebp_store;
        
        // register uint32_t eip asm("eip");
        printf(" \n \n EBP save: %d \n \n", ebp_save);
        printf(" \n \n ESP save: %d \n \n", esp_save);
        PCB->PID = PID;
        PCB->EBP = ebp_save;
        PCB->ESP = esp_save;
        prev_PID = PID;     //Have to save the current PID as the last PID
        current_process_idx = PID;
        PCB->fdesc_array.fd_entry[0].file_operations_table_pointer.read = t_read; //setting std in
        PCB->fdesc_array.fd_entry[1].file_operations_table_pointer.write = t_write; // setting std out



        printf("New prev PID: %d\n\n", prev_PID);
        // PCB->EIP = EIP_save;
        tss.esp0 = (EIGHT_MB - (PID)*EIGHT_KB) - 4; // updating the esp0

        // #define USER_CS     0x0023
        // #define USER_DS     0x002B

        
        asm volatile("pushl $0x002B"); // pushing User DS
        asm volatile("pushl $0x083ffffc"); //This Userspace stack pointer always starts here
        asm volatile("pushfl"); // this could be off -- pushing flags
        asm volatile("pushl $0x0023");  //pushing the USER CS
        asm volatile("pushl %0" : : "r" (EIP_save) : "memory");  // push EIP that was stored in the executable file
       
        asm volatile("iret ");

        // asm volatile("execute_to_halt:");

        // asm volatile("popl %eax"); // popping all of these off the stack -- that we just pushed
        // asm volatile("popl %eax"); 
        // asm volatile("popl %eax"); // this could be off -- pushing flags
        // asm volatile("popl %eax");  //pushing the USER CS
        // asm volatile("popl %eax");

        //the registers were all pushed originally, we'll se what happens

    printf("\n Active Process Number: %d", PCB->PID);
    printf("\n Base Pointer: %d", PCB->EBP);
    // printf("\n Instruction Pointer: %d", PCB->EIP);
    // printf("\n Active Process Number: %d", PCB->PID);

    //Potentially push eip, ask for more info later

    // printf("\n SYSCALL *EXECUTE* CALLED (SHOULD CORRESPOND TO SYSCALL 2)\n\n");

    //WILL NEED TO CHANGE THE return value that THE SYSTEM
    return retval;
}


int32_t sys_read() {
    int32_t fd;
    void * buf;
    int32_t nbytes;
    asm volatile("\t movl %%ebx,%0" : "=r"(fd)); // This line basically takes a value in a register and puts it into the variable
    asm volatile("\t movl %%ecx,%0" : "=r"(buf)); // This line basically takes a value in a register and puts it into the variable
    asm volatile("\t movl %%edx,%0" : "=r"(nbytes)); // This line basically takes a value in a register and puts it into the variable

    return (* PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.read)(fd, buf, nbytes);

    // printf("calling puts \n");
    // puts(buf);

    // printf("SYSCALL *READ* CALLED (SHOULD CORRESPOND TO SYSCALL 3)\n\n");
    // int i;
    // for(i=0; i<5; i++){
        
    // } 
    // return 0;
}

int32_t sys_write() {
    int32_t fd;
    void * buf;
    int32_t nbytes;

    asm volatile("\t movl %%ebx,%0" : "=r"(fd)); // This line basically takes a value in a register and puts it into the variable
    asm volatile("\t movl %%ecx,%0" : "=r"(buf)); // This line basically takes a value in a register and puts it into the variable
    asm volatile("\t movl %%edx,%0" : "=r"(nbytes)); // This line basically takes a value in a register and puts it into the variable
    // printf("SYSCALL *WRITE* CALLED (SHOULD CORRESPOND TO SYSCALL 4)\n\n");

    int32_t retval = (* PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.write)(fd, buf, nbytes);
    if(retval == -1){
        return -1;
    }
    return 0;
}


#define     FAILURE         -1
//Hypothetically works, haven't been able to check if EAX is auto populated with 
int32_t sys_open() {
    printf("\n made it to sys open \n ");
    //Working on sys_open
    //first we need to somehow get the argument (file name from the registers)
    //Then we need to call our old file open which gives us the dentry
    //allocate for a file descriptor, page table???
    // will need to have checks for whenever the file descriptor is full
    int8_t * filename;
    
    asm volatile("\t movl %%ebx,%0" : "=r"(filename)); // This line basically takes a value in a register and puts it into the variable
    int i;
    int fd_index_to_open;
    for(i = FD_START; i < FD_END; i++){ // find an open fd
        if(PCB_array[current_process_idx]->fdesc_array.fd_entry[i].flags == 0){ // if it's not occupied, set the file index we are going to use
            fd_index_to_open = i;
            break;
        }
    }


    
    /* POTENITAL RACE CONDITION FOR CHECKPOINT 5, we need to make sure that only one process can claim a given file, etc*/
    // fd_array.fd_entry[fd_index_to_open].file_operations_table_pointer = 0; // it's prob unnecessary to initialize these all to zero, but if we do it here we don't -past david

    //only 1 cpu lol -Future David


    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_position = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].inode = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].flags = 1; // declaring a fd 

    //fd_array.fd_entry[fd_index_to_open].file_operations_table_pointer[0];
    // printf("\n made it to line 881 \n");
    // // printf("\n If everything is correct, this should print out the file name: %d", filename);
    dentry_struct_t file_to_open;
    int32_t retval = read_dentry_by_name((uint8_t *)filename, &file_to_open);
    if(retval == FAILURE){
        printf("\n Sys Open Failed \n");
        return FAILURE;
    }
    if(file_to_open.file_type == RTC_FILE){
        //It is an RTC file
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open = (void *)rtc_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close = (void *)rtc_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)rtc_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)rtc_functions[3];
        int32_t rtc_open_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(rtc_open_stat == FAILURE){
            printf("\n Sys Open Failed -- RTC_Open \n");
            return FAILURE;
        }
    }else if(file_to_open.file_type == DIRECTORY_FILE){
        //it is a directory
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open =  (void *)directory_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close =  (void *)directory_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)directory_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)directory_functions[3];
        int32_t dopen_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(dopen_stat == FAILURE){
            printf("\n Sys Open Failed -- D_Open \n");
            return FAILURE;
        }
        
        //printf(" \n\n DIRECTORY Janky directory jumptable called here!\n\n");
        //asm volatile("jmp directory_jumptable");
    }else{
        //It is a file
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open =  (void *)file_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close =  (void *)file_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)file_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)file_functions[3];
        int32_t fopen_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(fopen_stat == FAILURE){
            printf("\n Sys Open Failed -- F_Open \n");
            return FAILURE;
        }
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].inode = file_to_open.inode_number;     // setting inode number
    }
    // printf("\n made it to line 905 \n");
    //temporary dentry has been allocated, gives us the file type and the inode number, useful for our jumptable which keeps track of various file operations dir read vs file read

    printf("\n SYSCALL *OPEN* CALLED (SHOULD CORRESPOND TO SYSCALL 5)\n\n");
    //PCB->fdesc_array = fd_array;
    return 0;
}

int32_t sys_close() {
    int32_t fd;
    asm volatile("\t movl %%ebx,%0" : "=r"(fd)); // This line basically takes a value in a register and puts it into the variable
    if(PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].flags == 0){
        printf("\n Attempted to Close Something that was not open in the first place \n");
        return FAILURE;
    } //zero out the fd
    int32_t retval = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.close)(fd);
    if(retval == FAILURE){
        return FAILURE;
    }
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.open = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.close = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.read = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.write = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].inode = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].flags = 0;
    

    printf("SYSCALL *CLOSE* CALLED (SHOULD CORRESPOND TO SYSCALL 6)\n\n");
    return 0;
}

int32_t sys_getargs() {
    uint8_t * buf;
    int32_t nbytes;
    asm volatile("\t movl %%ebx,%0" : "=r"(buf)); // This line basically takes a value in a register and puts it into the variablle
    asm volatile("\t movl %%ecx,%0" : "=r"(nbytes)); // This line basically takes a value in a register and puts it into the variable
    printf("SYSCALL *GETARGS* CALLED (SHOULD CORRESPOND TO SYSCALL 7)\n\n");
    return 0;
}

int32_t sys_vidmap() {
    uint8_t ** screen_start;
    asm volatile("\t movl %%ebx,%0" : "=r"(screen_start)); // This line basically takes a value in a register and puts it into the variablle
    printf("SYSCALL *VIDMAP* CALLED (SHOULD CORRESPOND TO SYSCALL 8)\n\n");
    return 0;
}

int32_t sys_set_handler() {
    int32_t signum;
    void * handler_address;

    asm volatile("\t movl %%ebx,%0" : "=r"(signum)); // This line basically takes a value in a register and puts it into the variablle
    asm volatile("\t movl %%ecx,%0" : "=r"(handler_address)); // This line basically takes a value in a register and puts it into the variablle
    printf("SYSCALL *SET_HANDLER* CALLED (SHOULD CORRESPOND TO SYSCALL 9)\n\n");
    return 0;
}

int32_t sys_sigreturn() {
    //no args i think? --dvt
    printf("SYSCALL *SIGRETURN* CALLED (SHOULD CORRESPOND TO SYSCALL 10)\n\n");
    return 0;
}
int32_t sys_error(){
    printf("\n Something went wrong, It is possible, the wrong system call index was provided. \n");
    return -1;
}










// for future
// handler for interrupt calls -- James
// gonna make the IDT entry for non-maskable interrupt point to this.
// for some reason the nmi interrupt vector previously redirected to sys_handler -- Wyatt
void intr_handler() {
    // asm("pushal") ;
    // asm("pushfl");
    printf("This is an intr handler moment");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    while(1){}
}



// void rtc_handler(){
//     //test_interrupts();
//     outb(0x0C, 0x70);	 //select register C
//     inb(0x71);		 //just throw away contents
//     //clear();
//     send_eoi(8);
// }

#define         RESERVED4MASK               0x1F // kill bits 7-5
#define         NUMBER_OF_VECTORS           256
#define         NUMBER_OF_EXCEPTIONS_DEFINING        20      //based on what the CA said, it seems like we only need these 20 exceptions 
#define         NUMBER_OF_SYS_CALLS         10 // for letter check points, for now we just have a simply handle for sys calls -- James




/*
*   Description:    This function initializes the external array idt that is defined in x86_desc.h and fills in the idt with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt array will be set and ready to be imported into the idt table that is defined in x86_desc.S
* 
*/
void initialize_idt(){ // need to set all 256 to something, zero everything out and then specify the ones we care about

    //printf("\n THE INITIALIZE IDT FUNCTION IS BEING CALLED \n");
    int i;
    //int j;
    for(i=0; i< NUMBER_OF_VECTORS; i++){ // Initially zero out every single vector in the idt
        idt[i].val[0] = 0x00000000;
        idt[i].val[1] = 0x00000000;
    }
    for(i=0;i <= NUMBER_OF_EXCEPTIONS_DEFINING; i++){
        // 15th exception is reserved so set present to 0
        if (i == 15 || i == 20) {
           (&(idt[i]))->present = 0;
        } else {
            set_exception_params(&idt[i], i);
        }
    }
    // while(1){
    //     for(j=0; j< 20; j++){
    //         printf("\n idt number %d : value: %d   \n", j, (int) idt[j].offset_15_00);
    //     }
    // }
    // for 0x80 sys call for now, we jsut acknowledge it in our sys handler
    // change the fields like in the function below for this sys call 
    idt_desc_t * idt_array_index = &(idt[0x80]);
    idt_array_index->seg_selector = KERNEL_CS; //This represents the kernel CS <- i think this is defined in x86_desc?
    idt_array_index->reserved4 = 0;
    idt_array_index->reserved3 = 1; // 0 corresponds to interrupt, 1 is trap
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 3; // this one is also going to depend on syscall vs trap/interrupt
    idt_array_index->present = 1; // 90% sure this bit needs to be 1 or else it won't like the address
    SET_IDT_ENTRY((*idt_array_index), sys_call);


    //SETTING UP THE KEYBOARD FOR DEVICES
    idt_array_index = &(idt[0x21]);
    // while(1){
    //     printf("\n We are setting up the keyboard here at idt init \n");
    // }
    idt_array_index->seg_selector = KERNEL_CS; //This represents the kernel CS <- i think this is defined in x86_desc?
    idt_array_index->reserved4 = 0;
    idt_array_index->reserved3 = 0; // 0 corresponds to interrupt, 1 is trap
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 3; // this one is also going to depend on syscall vs trap/interrupt
    idt_array_index->present = 1; // 90% sure this bit needs to be 1 or else it won't like the address
    SET_IDT_ENTRY((*idt_array_index), keyboard_call);
    // while(1){
    //     printf("\n We are setting up the keyboard here at idt init \n");
    // }
    //SETTING UP THE RTC FOR THE HANDLER
    idt_array_index = &(idt[0x28]);
    idt_array_index->seg_selector = KERNEL_CS; //This represents the kernel CS <- i think this is defined in x86_desc?
    idt_array_index->reserved4 = 0;
    idt_array_index->reserved3 = 0; // 0 corresponds to interrupt, 1 is trap
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 0; // this one is also going to depend on syscall vs trap/interrupt
    idt_array_index->present = 1; // 90% sure this bit needs to be 1 or else it won't like the address
    SET_IDT_ENTRY((*idt_array_index), rtc_call);
}

/*
*   Description:    This function initializes the external array idt[0->20] that is defined in x86_desc.h and fills in these exceptions with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt[0->20] array will be set and ready to be imported into the idt table that is defined in x86_desc.S
*   https://wiki.osdev.org/Exceptions // FILL THESE VALUES IN WITH THE INFO FROM THIS
*/      
void set_exception_params(idt_desc_t * idt_array_index, int vec){
    //int j;
    // do the below for every interrupt -- James 
    // set dpl to 0 for hardware interrupts and exception
    // sys call's dpl should be set to 3
    // set all segment selector to kernel_CS descriptor
    // size is always 1
    // present is always 1 (I think? Only 99% sure)
    // for reserve3...0, remember it's 0 size 1 1 0 <- intr gate, 0 size 1 1 1 <- trap gate
    //      for the 20 exceptions, only vector nr 2 (non-maskable interrupt) is intr gate, others are trap gate
    // to call SET_IDT_ENTRY make sure we use the right macro as there are interrupts
    // that have error code and we must pop if off. refer to intel doc to what has error code
    switch(vec) {
    case 0:
        SET_IDT_ENTRY((*idt_array_index), de);
        break;
    case 1:
        SET_IDT_ENTRY((*idt_array_index), db);
        break;
    case 2:
        SET_IDT_ENTRY((*idt_array_index), nmi);
        break;
    case 3:
        SET_IDT_ENTRY((*idt_array_index), bp);
        break;
    case 4:
        SET_IDT_ENTRY((*idt_array_index), of);
        break;
    case 5:
        SET_IDT_ENTRY((*idt_array_index), br);
        break;
    case 6:
        SET_IDT_ENTRY((*idt_array_index), ud);
        break;
    case 7:
        SET_IDT_ENTRY((*idt_array_index), nm);
        break;
    case 8:
        SET_IDT_ENTRY((*idt_array_index), df);
        break;
    case 9:
        SET_IDT_ENTRY((*idt_array_index), cso);
        break;
    case 10:
        SET_IDT_ENTRY((*idt_array_index), ts);
        break;
    case 11:
        SET_IDT_ENTRY((*idt_array_index), np);
        break;
    case 12:
        SET_IDT_ENTRY((*idt_array_index), ss);
        break;
    case 13:
        SET_IDT_ENTRY((*idt_array_index), gp);
        break;
    case 14:
        SET_IDT_ENTRY((*idt_array_index), pf);
        break;
    case 15:
        SET_IDT_ENTRY((*idt_array_index), mf);
        break;
    case 16:
        SET_IDT_ENTRY((*idt_array_index), mf);
        break;
    case 17:
        SET_IDT_ENTRY((*idt_array_index), ac);
        break;
    case 18:
        SET_IDT_ENTRY((*idt_array_index), mc);
        break;
    case 19:
        SET_IDT_ENTRY((*idt_array_index), xf);
        break;
    default:
        // Nothing here for default case
        break;
}


    idt_array_index->seg_selector = KERNEL_CS; //This represents the kernel CS <- i think this is defined in x86_desc?
    // idt_array_index->reserved4 = idt_array_index->reserved4 &  RESERVED4MASK;
    idt_array_index->reserved4 = 0;
    // if(vec == 2 ){ // VEC =2 is NMI meaning it is an interrupt, but otherwise the exceptions are traps 
    //     idt_array_index->reserved3 = 0; // 0 corresponds to interrupt
    // }else{
    idt_array_index->reserved3 = 1; // 1 corresponds to trap gate
    // }
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 0; // this one is also going to depend on syscall vs trap/interrupt
    idt_array_index->present = 1; // 90% sure this bit needs to be 1 or else it won't like the address
    
    // while(1){ 
    //     for(j=0; j< 20; j++){
    //         printf("\n idt number %d : value: %d   \n", j, (int) idt[j].offset_15_00);
    //     }
    // }
}
//GOING TO HAVE A FUNCTION THAT FILLS IN A MAJORITY OF THE STRUCT, probs will take a few parameters and then change the things that need to be changed


