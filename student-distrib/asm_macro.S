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

#define MY_ASM_MACRO_KEYBOARD(function_name, handler, vector) \
        .global function_name ;\
        function_name: ;\
        movl (%esp), %eax ;\
        movl %eax, EIP_SAVE ;\
        movl 4(%esp), %eax ;\
        movl %eax, CS_SAVE ;\
        movl 8(%esp), %eax ;\
        movl %eax, EFLAGS_SAVE ;\
        movl 12(%esp), %eax ;\
        movl %eax, ESP_SAVE ;\
        movl 16(%esp), %eax ;\
        movl %eax, SS_SAVE ;\
        pushal ;\
        pushfl ;\
        pushl %ebp ;\
        movl SS_SAVE, %eax ;\
        push %eax ;\
        movl ESP_SAVE, %eax ;\
        push %eax ;\
        movl EFLAGS_SAVE, %eax ;\
        push %eax ;\
        movl CS_SAVE, %eax ;\
        push %eax ;\
        movl EIP_SAVE, %eax ;\
        pushl %eax ;\
        call handler ;\
        addl $24, %esp ;\
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
        cmp $0, %eax ;\
        jle ERROR ;\
        cmp $10, %eax ;\
        jg ERROR ;\
        jmp *jumptable(,%eax,4) ;\



MY_ASM_MACRO(de, exec_handler0, 0); // THIS MIGHT NEED TO BE FIXED LATER
MY_ASM_MACRO(db, exec_handler1, 1);
MY_ASM_MACRO(nmi, intr_handler, 2); //Wyatt -- changed this line to intr_handler, old handler was sys_call. Hopefully this doesn't break anything
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
MY_ASM_MACRO_KEYBOARD(keyboard_call, kb_handler, 0x21);  //handles keyboard interrupts
MY_ASM_MACRO(rtc_call, rtc_handler, 0x28);      //handles rtc interrupts






# Wyatt -- added Addendum B system call labels below this point. i think this will work.


EIP_SAVE:
    .long 0
CS_SAVE:
    .long 0
EFLAGS_SAVE:
    .long 0
ESP_SAVE:
    .long 0
SS_SAVE:
    .long 0

storage:
    .long 0


jumptable:
    .long ERROR
    .long HALT
    .long EXECUTE
    .long READ
    .long WRITE
    .long OPEN
    .long CLOSE
    .long GETARGS
    .long VIDMAP
    .long SET_HANDLER
    .long SIGRETURN


HALT:
    movl %ebx, %eax
    xorl %ebx, %ebx
    movb %al, %bl
    pushl %ebx
    call sys_halt
    popl %ebx  
    jmp FINISH

EXECUTE:   
    pushl %ebx
    call sys_execute
    popl %ebx
    jmp FINISH 

READ:   
    pushl %edx
    pushl %ecx
    pushl %ebx
    call sys_read
    popl %ebx
    popl %ecx
    popl %edx
    jmp FINISH 

WRITE:   
    pushl %edx
    pushl %ecx
    pushl %ebx
    call sys_write
    popl %ebx
    popl %ecx
    popl %edx
    jmp FINISH 

OPEN:   
    pushl %ebx
    call sys_open
    popl %ebx
    jmp FINISH

CLOSE:   
    pushl %ebx
    call sys_close
    popl %ebx
    jmp FINISH 

GETARGS:   
    pushl %ecx
    pushl %ebx
    call sys_getargs
    popl %ebx
    popl %ecx
    jmp FINISH 

VIDMAP:   
    pushl %ebx
    call sys_vidmap
    popl %ebx
    jmp FINISH 

SET_HANDLER:   
    pushl %ecx
    pushl %ebx
    call sys_set_handler
    popl %ebx
    popl %ecx
    jmp FINISH

SIGRETURN:   
    call sys_sigreturn
    jmp FINISH 

ERROR:
    call sys_error
    jmp FINISH
    
FINISH:     
    popfl
    movl %eax , storage 
    popal
    movl storage, %eax 
    iret 






