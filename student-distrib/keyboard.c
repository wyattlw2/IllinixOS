//keyboard is IRQ1

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "types.h"

#define     KEYBOARD_PORT       0x60
#define     PIC_MASTER          0X21
#define     IRQ1                0x01
#define     NUM_COLS            80
// lookup table that only does numbers and letters
// for scancodes other than 
const char table[] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
, '0', '\0', '\0', '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
'p', '\0', '\0', '\n', '\0', 'a', 's', 'd', 'f', 'g', 'h' , 'j', 'k' ,'l', '\0'
, '\0', '\0', '\0', '\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm'};


/*
init keyboard

Description: This function enables the irq line for the keyboard, effectively initializing it.
Inputs: None
Outputs: None
Side effects: Keyboard is initialized

*/
void init_keyboard(){
    clear();
    enable_cursor(0, 25);
    update_cursor(0, 0);
    update_xy(0, 0);
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

/* void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
 * Inputs:  uint8_t cursor_start - what line cursor should start
 *          uint8_t cursor_end - what line it should end
 * Return Value: none
 * Function: enables cursor*/
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x0A , 0x3D4);
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
	outb(0x0B, 0x3D4);
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}

/* int disable_cursor()
 * Inputs:  none
 * Return Value: none
 * Function: closes cursor*/
void disable_cursor() {
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}

/* update_cursor(int x, int y)
 * Inputs:  x - x-coordinate to update cursor to
 *          y - y-coordinate to update cursor to
 * Return Value: none
 * Function: closes cursor */
void update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;
	outb(0x0F,0x3D4);
	outb((uint8_t) (pos & 0xFF),0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF),0x3D5);
}
/* get_cursor_position(void)
 * Inputs:  void
 * Return Value: pos
 * Function: returns position of the cursor */
uint16_t get_cursor_position(void) {
    uint16_t pos = 0;
    outb(0x0F, 0x3D4);
    pos |= inb(0x3D5);
    outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}
