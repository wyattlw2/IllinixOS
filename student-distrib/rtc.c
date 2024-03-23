#include "rtc.h"
#include "types.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"

 void init_rtc(void){ //as154
     // initializing the RTC
    enable_irq(8);

    // set registers for RTC
    //cli();  //clear interrupts
    //outb(0x8A, 0x70);	 //select Status Register A, and disable NMI (by setting the 0x80 bit)
    //outb(0x20, 0x71);	 //write to CMOS/RTC RAM got it
    //sti();		 //enable interrupts
    
    //turn on irq8
    //cli();  //clear interrupts
    outb(0x8B, inb(0x70) | 0x80); //select register B, and disable NMI
    char prev = inb(0x71);	 //read the current value of register B
    outb(0x8B, 0x70);		 //set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);	 //write the previous value ORed with 0x40. This turns on bit 6 of register B
    //sti();  //enable interrupts
    rtc_int = 0;
 }

// void init_rtc_DAVID_ATTEMPT(){


//     outb(0x8B, 0x70);		// select register B, and disable NMI
//     char prev = inb(0x71);	// read the current value of register B
//     outb(0x8B, 0x70);		// set the index again (a read will reset the index to register D)
//     outb(prev | 0x40, 0x71);

//     outb(0x70, 0x0C);	// select register C
//     inb(0x71);		// just throw away contents
// }

