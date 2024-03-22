#include "rtc.h"

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
     outb(0x8b, 0x70);		 //select register B, and disable NMI
     char prev = inb(0x71);	 //read the current value of register B
     outb(0x8b, 0x70);		 //set the index again (a read will reset the index to register D)
     outb(prev | 0x40, 0x71);	 //write the previous value ORed with 0x40. This turns on bit 6 of register B
     //sti();  //enable interrupts

    
    //  //changing interrupt rates
    //  unsigned int rate = 6;  
    //  rate &= 0x0F;            //rate must be above 2 and not over 15
    //  //cli();
    //  outb(0x8A, 0x70);		 //set index to register A, disable NMI
    //  prev = inb(0x71);	 //get initial value of register A
    //  outb(0x8A, 0x70);		 //reset index to A
    //  outb((prev & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.
    //  //sti();

      //must read register C so interrupt can happen again
     //outb(0x0C, 0x70);	 //select register C
     //inb(0x71);		 //just throw away contents
 }


void init_rtc_DAVID_ATTEMPT(){


    outb(0x8B, 0x70);		// select register B, and disable NMI
    char prev = inb(0x71);	// read the current value of register B
    outb(0x8B, 0x70);		// set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);

    outb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents
}



/* //as154
int32_t rtc_set_frequency(int32_t frequency){

    return 0;
}

int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){


    return 0;
}

int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes){


    return 0;
}

int32_t rtc_open (const uint8_t* filename){
    rtc_set_frequency(2); //Initialize RTC Frequency to 2 HZ
    return 0; //Return zero for success 
}

int32_t rtc_close (int32_t fd){
    return 0;
}
*/
