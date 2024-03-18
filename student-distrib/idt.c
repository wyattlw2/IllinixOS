




#include "idt.h"
#include "lib.h"

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


extern void de(void);
extern void db(void);
extern void nmi(void);
extern void bp(void);
extern void of(void);
extern void br(void);
extern void ud(void);
extern void nm(void);
extern void df(void);
extern void cso(void);
extern void ts(void);
extern void np(void);
extern void ss(void);
extern void gp(void);
extern void pf(void);
extern void mf(void);
extern void ac(void);
extern void mc(void);
extern void xf(void);
extern void sys_call(void);


/*
*   Description:    This function initializes the external array idt that is defined in x86_desc.h and fills in the idt with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt array will be set and ready to be imported into the idt table that is defined in x86_desc.S
*
*/
//20 exceptions on osdev
//rtc, keyboard , PIT (for scheduling comes later)

// handler for exceptions -- James
void exec_handler(int vec) {
    printf(vec);
    while(1){}
}
// handler for sys_calls -- James
void sys_handler(int vec) {
    printf(vec);
    while(1){}
}
// for future
// handler for interrupt calls -- James
void intr_handler(int vec) {
    printf(vec);
    while(1){}
}

#define         NUMBER_OF_VECTORS           256
#define         NUMBER_OF_EXCEPTIONS_DEFINING        20      //based on what the CA said, it seems like we only need these 20 exceptions 
#define         NUMBER_OF_SYS_CALLS         10 // for letter check points, for now we just have a simply handle for sys calls -- James
void initialize_idt(){ // need to set all 256 to something, zero everything out and then specify the ones we care about
    int i;


    
    for(i=0; i< NUMBER_OF_VECTORS; i++){ // Initially zero out every single vector in the idt
        idt[i].val[0] = 0x00000000;
        idt[i].val[1] = 0x00000000;
    }
    for(i=0;i< NUMBER_OF_EXCEPTIONS_DEFINING; i++){
        set_exception_params(idt[i], i);
    }
    // for 0x80 sys call for now, we jsut acknowledge it in our sys handler
    // change the fields like in the function below for this sys call 
    SET_IDT_ENTRY(idt[0x80], MY_ASM_MACRO(t, exec_handler, vec));    // not sure if this is how you actually pass to the macro
                                                                            // also dk how to call set_idt_entry correctly? feel free to help-- James 
}

/*
*   Description:    This function initializes the external array idt[0->20] that is defined in x86_desc.h and fills in these exceptions with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt[0->20] array will be set and ready to be imported into the idt table that is defined in x86_desc.S
*   https://wiki.osdev.org/Exceptions // FILL THESE VALUES IN WITH THE INFO FROM THIS
*/      
void set_exception_params(idt_desc_t idt_array_index, int vec){
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
    if (vec == 8 || vec == 10 || vec == 11 || vec == 12 || vec == 13 || vec == 14 || vec == 17) {
        SET_IDT_ENTRY(idt_array_index, MY_ASM_MACRO_ERR_CODE(t, exec_handler, vec));    // not sure if this is how you actually pass to the macro
                                                                                // also dk how to call set_idt_entry correctly? feel free to help-- James
    } else {
        SET_IDT_ENTRY(idt_array_index, MY_ASM_MACRO(t, exec_handler, vec));    // not sure if this is how you actually pass to the macro
                                                                                // also dk how to call set_idt_entry correctly? feel free to help-- James
    }
    
}
//GOING TO HAVE A FUNCTION THAT FILLS IN A MAJORITY OF THE STRUCT, probs will take a few parameters and then change the things that need to be changed