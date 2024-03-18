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
        addl $8, %esp ;\
        popfl ;\
        popal ;\
        iret ;\