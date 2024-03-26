//#include "asm_macro.h"
//#include "idt.h"

// sets up the necessary infrastructure for exception handling
// our actual handler contains the logic for handling different exceptions -- James
#define MY_ASM_MACRO(function_name, handler, vector) \
    .global function_name ;\
    function_name: ;\
        pushal ;\
        pushfl ;\
        pushl $vector ;\
        call handler ;\
        addl $4, %esp ;\
        popfl ;\
        popal ;\
        iret ;\
// same macro as above but for interrupts with error code -- James
#define MY_ASM_MACRO_ERR_CODE(function_name, handler, vector) \
    .global function_name ;\
    function_name: ;\
        pushal ;\
        pushfl ;\
        pushl $vector ;\
        call handler ;\
        addl $4, %esp ;\
        popfl ;\
        popal ;\
        iret ;\

#define MY_ASM_MACRO_SYS_CALL(function_name) \
    .global function_name ;\
    function_name: ;\
        pushal ;\
        pushfl ;\
        jmp *jumptable(,%eax,4) ;\



MY_ASM_MACRO(de, exec_handler0, 0); // THIS MIGHT NEED TO BE FIXED LATER
MY_ASM_MACRO(db, exec_handler1, 1);
MY_ASM_MACRO(nmi, sys_handler, 2);
MY_ASM_MACRO(bp, exec_handler3, 3);
MY_ASM_MACRO(of, exec_handler4, 4);
MY_ASM_MACRO(br, exec_handler5, 5);
MY_ASM_MACRO(ud, exec_handler6, 6);
MY_ASM_MACRO(nm, exec_handler7, 7);
MY_ASM_MACRO_ERR_CODE(df, exec_handler8, 8);
MY_ASM_MACRO(cso, exec_handler9, 9);
MY_ASM_MACRO_ERR_CODE(ts, exec_handler10, 10);
MY_ASM_MACRO_ERR_CODE(np, exec_handler11, 11);
MY_ASM_MACRO_ERR_CODE(ss, exec_handler12, 12);
MY_ASM_MACRO_ERR_CODE(gp, exec_handler13, 13);
MY_ASM_MACRO_ERR_CODE(pf, exec_handler14, 14);
// 15 is reserved
MY_ASM_MACRO(mf, exec_handler16, 16);
MY_ASM_MACRO_ERR_CODE(ac, exec_handler17, 17);
MY_ASM_MACRO(mc, exec_handler18, 18);
MY_ASM_MACRO(xf, exec_handler19, 19);
// 20 is reserved
MY_ASM_MACRO_SYS_CALL(sys_call);
MY_ASM_MACRO(keyboard_call, kb_handler, 0x21);  //handles keyboard interrupts
MY_ASM_MACRO(rtc_call, rtc_handler, 0x28);      //handles rtc interrupts



jumptable_dispatcher:
    pushal
    pushfl
    jmp *jumptable(,%eax,4)

jumptable:  
    .long OPEN 
    .long WRITE 
    .long READ 
    .long CLOSE 


OPEN:   
    
    call sys_handler    
    jmp FINISH

WRITE:   
    
    call sys_handler
    jmp FINISH 

READ:   
    
    call sys_handler
    jmp FINISH 

CLOSE:   
    
    call sys_handler
    jmp FINISH 


FINISH:     
    popfl 
    popal 
    iret 






