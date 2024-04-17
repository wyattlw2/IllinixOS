#include "scheduling.h"
// #include "types.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_FREQUENCY 1193182

#define DESIRED_FREQUENCY 100


/*  init_pit()

Description: Initializes PIT (we may or may not need this, prolly tho)
*/
void init_pit() {
    uint16_t divisor = (uint16_t) PIT_FREQUENCY / DESIRED_FREQUENCY;
    outb(0x36, PIT_COMMAND);
    outb((uint8_t)(divisor & 0xFF), PIT_CHANNEL0);
    outb((uint8_t)((divisor >> 8) & 0xFF), PIT_CHANNEL0);
    enable_irq(0);
    printf("PIT: Initialized\n");
}

// void timer_phase(int hz)
// {
//     int divisor = 1193180 / hz;       /* Calculate our divisor */
//     outportb(0x43, 0x36);             /* Set our command byte 0x36 */
//     outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
//     outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
// }
/*  pit_handler()

Description: Handles PIT interrupts. We're able to do scheduling inside of the PIT, or alternatively we can make a separate scheduling function. Would prefer the latter option
*/
void pit_handler()  {
    printf("\n PIT INTERRUPT");
    send_eoi(0);
    // sti();
    return;
}

