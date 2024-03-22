#include "rtc.h"
#include "types.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"

int rtc_int;

/* OSDEV NMI Enable & Disable
void NMI_enable(){
    outb(0x70. inb(0x70) & 0x7F);
    inb(0x71);
}
void NMI_disable(){
    outb(0x70, inb(0x70) | 0x80);
    inb(0x71);
}
*/

/*
* Description: 
* Inputs: 
* Outputs: 
* Side Effects: 
*/
 void init_rtc(void){ // initializing the RTC -- Aadhesh
    enable_irq(8);
    //cli() called prior to init_rtc()
    char prev = inb(0x70) | 0x80; //Disable NMI by setting first bit (0x80 bit) to 1 using 0x80
    outb(prev, 0x70);
    inb(0x71);

    outb(0x8B, 0x70); //select register B, and disable NMI
    prev = inb(0x71); //read the current value of register B
    outb(0x8B, 0x70); //set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71); //write the previous value ORed with 0x40. This turns on bit 6 of register B

    rtc_int = 0; //Initialize RTC interrupt flag
 }

void rtc_handler(){
    cli();
    test_interrupts();
    rtc_int += 1;
    outb(0x0C, 0x70); //select register C
    inb(0x71); //just throw away contents
    //clear();
    send_eoi(8);
}

int32_t rtc_set_frequency(int32_t frequency){ //created to handle rtc_write and rtc_open -- Aadhesh
    //printf("%d", frequency);
    if(frequency < 2 || frequency > 1024) return -1; //Return failure if frequency exceeds 1024 Hz or drops below 2 Hz
    if(frequency & (frequency - 1)) return -1; //Check that frequency is power of 2 (bitwise-and frequency and frequency -1; if power of 2, bitwise operation returns 0)

    //changing interrupt rates
    int log = 0;
    while(frequency >>= 1) log++; //log2(frequency)
    uint8_t rate = 16 - log; //rate must be above 2 and not over 15

    cli(); //clear interrupts
    char prev = inb(0x70) | 0x80; //Disable NMI by setting first bit (0x80 bit) to 1 using 0x80
    outb(prev, 0x70);
    inb(0x71); 

    outb(0x8A, 0x70); //set index to register A
    prev = inb(0x71); //get initial value of register A
    outb(0x8A, 0x70); //reset index to A
    outb((prev & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.
    
    prev = inb(0x70) & 0x7F; //Enable NMI by setting 0x80 bit to 0 using 0x7F
    outb(prev,0x70);
    inb(0x71);
    sti(); //Enable all other interrupts

    return 0;
}

int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){ //Aadhesh
    int32_t curr = rtc_int; //Stores current rtc
    while(curr == rtc_int){}; //Waits until rtc_int value is changed by the rtc_handler
    return 0;
}

int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes){ //Aadhesh
    if(nbytes != 4) return -1; //Return failure if more or less than 4 bytes passed in
    return rtc_set_frequency((int32_t)buf); //change frequency based on buf value 
}

int32_t rtc_open (const uint8_t* filename){ //Aadhesh
    rtc_set_frequency(2); //Initialize RTC Frequency to 2 Hz
    return 0; //Return zero for success 
}

int32_t rtc_close (int32_t fd){ //Aadhesh
    return 0;
}