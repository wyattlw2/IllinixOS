//keyboard is IRQ1

#include "lib.h"
#include "i8259.h"'
#include "types.h"
#define     KEYBOARD_PORT       0x60;
#define     PIC_MASTER          0X21;
#define     IRQ1                0x01;
// lookup table that only does numbers and letters
// for scancodes other than 
const char table[] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
, '0', '\0', '\0', '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
'p', '\0', '\0', '\n', '\0', 'a', 's', 'd', 'f', 'g', 'h' , 'j', 'k' ,'l', '\0'
, '\0', '\0', '\0', '\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm'};

void initialize_keyboard(){
    //  Enable PIC Mask x01
    enable_irq(1);
    // uint32_t mask = inb(PIC_MASTER);        // read current value
    // mask &= ~(1 << IRQ1);                   // clear all the bit but the IRQ1 bit
    // outb(PIC_MASTER, mask);                 
    // --- KEYBOAARD HANDLER??
    //  create table for scancodes
    //  accept scancode with inb
    //  print it to the screen
};
// make an interrupt specifically for the keyboard
//do the classic int $21 or whatever to go into the keyboard handler
//How to know where to put int $21 or whatever