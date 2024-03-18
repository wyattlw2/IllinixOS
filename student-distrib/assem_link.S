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

MY_ASM_MACRO(de, exec_handler, 0)
MY_ASM_MACRO(db, exec_handler, 1)
MY_ASM_MACRO(nmi, sys_handler, 2)
MY_ASM_MACRO(bp, exec_handler, 3)
MY_ASM_MACRO(of, exec_handler, 4)
MY_ASM_MACRO(br, exec_handler, 5)
MY_ASM_MACRO(ud, exec_handler, 6)
MY_ASM_MACRO(nm, exec_handler, 7)
MY_ASM_MACRO_ERR_CODE(df, exec_handler, 8)
MY_ASM_MACRO(cso, exec_handler, 9)
MY_ASM_MACRO_ERR_CODE(ts, exec_handler, 10)
MY_ASM_MACRO_ERR_CODE(np, exec_handler, 11)
MY_ASM_MACRO_ERR_CODE(ss, exec_handler, 12)
MY_ASM_MACRO_ERR_CODE(gp, exec_handler, 13)
MY_ASM_MACRO_ERR_CODE(pf, exec_handler, 14)
// 15 is reserved
MY_ASM_MACRO(mf, exec_handler, 16)
MY_ASM_MACRO_ERR_CODE(ac, exec_handler, 17)
MY_ASM_MACRO(mc, exec_handler, 18)
MY_ASM_MACRO(xf, exec_handler, 19)
// 20 is reserved
MY_ASM_MACRO(sys_call, sys_handler, 0x80)
