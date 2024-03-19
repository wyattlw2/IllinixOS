/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
#define PIC1				0x20		/* IO base address for master PIC */
#define PIC2				0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND		PIC1
#define PIC1_DATA			(PIC1+1)
#define PIC2_COMMAND		PIC2
#define PIC2_DATA			(PIC2+1)

#define MASTER_OFFSET 		ICW2_MASTER
#define SLAVE_OFFSET		ICW2_SLAVE

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */




/*
i8259_init()

Description: Initializes the PIC chip. Necessary for keyboard and RTC to work properly.
Inputs: None
Outputs: None
Side effects: Masks PIC and sends the required ICWs such that the PIC is initialized and ready to use with devices.
*/
/* Initialize the 8259 PIC */
void i8259_init(void) {

    
    // outb(PIC1_DATA, 0xff);
    // outb(PIC2_DATA, 0xff);

   
    //disable_irq(1);

    uint8_t a1, a2;
    a1 = inb(ICW2_MASTER);
    a2 = inb(ICW2_SLAVE);
	outb(ICW1 | ICW4, ICW2_MASTER);  // starts the initialization sequence (in cascade mode)
	outb(ICW1 | ICW4, ICW2_SLAVE);	
	outb(MASTER_OFFSET, PIC1_DATA);                 // ICW2: Master PIC vector offset
	outb(SLAVE_OFFSET, PIC2_DATA);                 // ICW2: Slave PIC vector offset
	outb(4, PIC1_DATA);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(2, PIC2_DATA);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
 
	outb(ICW4_8086, PIC1_DATA);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(ICW4_8086, PIC2_DATA);
 
	outb(a1, ICW2_MASTER);   // restore saved masks.
	outb(a2, ICW2_SLAVE);

    outb (0xff, PIC1_DATA);
   outb( 0xff, PIC2_DATA);

    // outb(ICW2_MASTER , ICW1 | ICW4);  // starts the initialization sequence (in cascade mode)
	// outb(ICW2_SLAVE, ICW1 | ICW4);	
	// outb(PIC1_DATA, MASTER_OFFSET);                 // ICW2: Master PIC vector offset
	// outb(PIC2_DATA, SLAVE_OFFSET);                 // ICW2: Slave PIC vector offset
	// outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	// outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
 
	// outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	// outb(PIC2_DATA, ICW4_8086);
 
	// outb(ICW2_MASTER, a1);   // restore saved masks.
	// outb(ICW2_SLAVE, a2);


}



/*
enable_irq

Description: Unmasks an IRQ line corresponding to the inputted IRQ number.
Inputs: irq_num
Outputs: None
Side effects: Unmasks IRQ line corresponding to irq_num
*/
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    // cli();
    uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        irq_num -= 8;
        port = PIC2_DATA;
    }
    value = inb(port) | (1 << irq_num);
    outb(value, port);  
    // outb(port, value);        
    // sti();
}


/*
disable_irq

Description: Masks the IRQ line corresponding to the inputted irq number
Inputs: irq_num
Outputs: None
Side effects: Masks the IRQ line corresponding to irq_num
*/
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    // cli();
	uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
    // outb(port, value);
    // sti();        
}


/*
send_eoi()

Description: Sends an EOI to the corresponding IRQ line to signify the ISR for that specific interrupt has finished
Inputs: irq_num
Outputs: None
Side effects: Sends an EOI to the corresponding IRQ line (based on irq_num) to signify the ISR for that specific interrupt has finished
*/
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    // cli();
	if(irq_num >= 8){
		outb(EOI | 2, PIC1_COMMAND);
		outb(EOI | (irq_num - 8), PIC2_COMMAND);
    }else{
    	outb(EOI | irq_num, PIC1_COMMAND);
    }

    // if(irq_num >= 8)
	// 	outb(PIC2_COMMAND, EOI);
 
	// outb(PIC1_COMMAND, EOI);
    // sti();
}
