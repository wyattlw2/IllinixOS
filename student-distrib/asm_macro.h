#ifndef ASM_FUNCTIONS_H
#define ASM_FUNCTIONS_H

#ifndef asm
void de();
void db();
void nmi();
void bp();
void of();
void br();
void ud();
void nm();
void df();
void cso();
void ts();
void np();
void ss();
void gp();
void pf();
void mf();
void ac();
void mc();
void xf();
void exec_c19();
void sys_call();
void keyboard_call();
void rtc_call();
void jumptable_dispatcher();

void pit_call();        //used for scheduling in CP5

void RTCJ();
void FILEJ();
void DIRJ();
#endif

#endif /* ASM_FUNCTIONS_H */

