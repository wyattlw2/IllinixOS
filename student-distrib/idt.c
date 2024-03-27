#include "idt.h"
#define     VIDEO               0xB8000
#define     KEYBOARD_PORT       0x60       //WYATT ADDED
#define     NUM_COLS            80
#define     NUM_ROWS            25
#define     MAX_BUFF_SIZE       128
#define     SPEC_CHAR_OFFSET    54
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
            kb_idx--;
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
        user_y += 2; // add 2 because we need to print the buffer value but also move to a new line

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
        user_y = 0;
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

void sys_halt() {
    //Is it recommended that we pass the arguments with registers
    // are we going to pass the arguments in user space or kernel space, how does it work, who calls who... etc
    //Is the assembly linkage given for us in this case --ece391 assem link


    //There is a switch statement here for all 10 syscalls for the file system
    //eax is going to have number -- Probably actually going to be a jumptable though



    printf("SYSCALL *HALT* CALLED (SHOULD CORRESPOND TO SYSCALL 1, WHICH IS THE FIRST SYSCALL IN ADDENDUM B)\n\n");
    // asm("popfl") ;
    // asm("popal") ;
    //asm("iret") ;
    //while(1){}
    return;
}

void sys_execute() {
    printf("SYSCALL *EXECUTE* CALLED (SHOULD CORRESPOND TO SYSCALL 2)\n\n");
    return;
}

void sys_read() {
    printf("SYSCALL *READ* CALLED (SHOULD CORRESPOND TO SYSCALL 3)\n\n");
    return;
}

void sys_write() {
    printf("SYSCALL *WRITE* CALLED (SHOULD CORRESPOND TO SYSCALL 4)\n\n");
    return;
}

void sys_open() {
    printf("SYSCALL *OPEN* CALLED (SHOULD CORRESPOND TO SYSCALL 5)\n\n");
    return;
}

void sys_close() {
    printf("SYSCALL *CLOSE* CALLED (SHOULD CORRESPOND TO SYSCALL 6)\n\n");
    return;
}

void sys_getargs() {
    printf("SYSCALL *GETARGS* CALLED (SHOULD CORRESPOND TO SYSCALL 7)\n\n");
    return;
}

void sys_vidmap() {
    printf("SYSCALL *VIDMAP* CALLED (SHOULD CORRESPOND TO SYSCALL 8)\n\n");
    return;
}

void sys_set_handler() {
    printf("SYSCALL *SET_HANDLER* CALLED (SHOULD CORRESPOND TO SYSCALL 9)\n\n");
    return;
}

void sys_sigreturn() {
    printf("SYSCALL *SIGRETURN* CALLED (SHOULD CORRESPOND TO SYSCALL 10)\n\n");
    return;
}
void sys_error(){
    printf("\n Something went wrong, It is possible, the wrong system call index was provided. \n");
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
    idt_array_index->reserved3 = 0; // 0 corresponds to interrupt, 1 is trap
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 0; // this one is also going to depend on syscall vs trap/interrupt
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
    
    idt_array_index->dpl = 0; // this one is also going to depend on syscall vs trap/interrupt
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


