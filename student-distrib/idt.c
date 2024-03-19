#include "idt.h"

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




/*
*   Description:    This function initializes the external array idt that is defined in x86_desc.h and fills in the idt with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt array will be set and ready to be imported into the idt table that is defined in x86_desc.S
*
*/
//20 exceptions on osdev
//rtc, keyboard , PIT (for scheduling comes later)

//uint32_t de, db, nmi, bp, of, br, ud, nm, df, cso, ts, np, ss, gp, pf, mf, ac, mc, xf, sys_call;

// handler for exceptions -- James
void exec_handler(int vec) {
    printf("Vector Number: ",vec);
    while(1){}
}
// handler for sys_calls -- James
void sys_handler(int vec) {
    printf("Vector Number: ",vec);
    while(1){}
}
// for future
// handler for interrupt calls -- James
void intr_handler(int vec) {
    printf("Vector Number: ",vec);
    while(1){}
}

#define         RESERVED4MASK               0x1F // kill bits 7-5
#define         NUMBER_OF_VECTORS           256
#define         NUMBER_OF_EXCEPTIONS_DEFINING        20      //based on what the CA said, it seems like we only need these 20 exceptions 
#define         NUMBER_OF_SYS_CALLS         10 // for letter check points, for now we just have a simply handle for sys calls -- James

#define KERNEL_CS_SEG_SELECTOR      0x0000
void initialize_idt(){ // need to set all 256 to something, zero everything out and then specify the ones we care about

    //printf("\n THE INITIALIZE IDT FUNCTION IS BEING CALLED \n");
    int i;
    int j;
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
    idt_array_index->seg_selector = KERNEL_CS_SEG_SELECTOR; //This represents the kernel CS <- i think this is defined in x86_desc?
    idt_array_index->reserved4 = idt_array_index->reserved4 &  RESERVED4MASK;
    idt_array_index->reserved3 = 1; // 0 corresponds to interrupt, 1 is trap
    idt_array_index->reserved2 = 1; // RESERVED BITS 0-2 are specified on intel's x86 documentation
    idt_array_index->reserved1 = 1;
    idt_array_index->size = 1; // Means we are in 32 bit mode
    idt_array_index->reserved0 = 0;
    
    idt_array_index->dpl = 0; // this one is also going to depend on syscall vs trap/interrupt
    idt_array_index->present = 1; // 90% sure this bit needs to be 1 or else it won't like the address
    SET_IDT_ENTRY((*idt_array_index), sys_handler);


    //int j;
    // while(1){
    //     for(j=0; j< 20; j++){
    //         printf("\n idt number %d : value: %d   \n", j, (int) idt[j].offset_15_00);
    //     }
    // }
}

/*
*   Description:    This function initializes the external array idt[0->20] that is defined in x86_desc.h and fills in these exceptions with what it needs to be filled in as
*   Inputs:         None
*   Outputs:        None
*   Side Effects:   The idt[0->20] array will be set and ready to be imported into the idt table that is defined in x86_desc.S
*   https://wiki.osdev.org/Exceptions // FILL THESE VALUES IN WITH THE INFO FROM THIS
*/      


void set_exception_params(idt_desc_t * idt_array_index, int vec){
    int j;
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

    SET_IDT_ENTRY((*idt_array_index), exec_handler); // this should fill in the offset fields
    // while(1){
    //     printf("\n This is the current offset of the idt index thing:  %d  \n", (int) idt_array_index->offset_31_16);
    // }
    // while(1){
    //     for(j=0; j< 20; j++){
    //         printf("\n idt number %d : value: %d   \n", vec, (int) idt[vec].offset_15_00);
    //     }
    // }

    idt_array_index->seg_selector = KERNEL_CS_SEG_SELECTOR; //This represents the kernel CS <- i think this is defined in x86_desc?
    idt_array_index->reserved4 = idt_array_index->reserved4 &  RESERVED4MASK;
    if(vec == 2 ){ // VEC =2 is NMI meaning it is an interrupt, but otherwise the exceptions are traps 
        idt_array_index->reserved3 = 0; // 0 corresponds to interrupt
    }else{
        idt_array_index->reserved3 = 1; // 1 corresponds to trap gate
    }
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

