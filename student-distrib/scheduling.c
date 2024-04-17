#include "scheduling.h"
// #include "types.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_FREQUENCY 1193182

#define DESIRED_FREQUENCY 100




#define NUM_ROWS    25
#define NUM_COLS    80
#define ATTRIB      0xA

static char * video_mem1 = (char *) 0xBA000;
static char * video_mem2 = (char *) 0xBB000;
static char * video_mem3 = (char *) 0xBC000;
void terminal_init(){
    no_parent_shell_flag = 1; // WE DO THIS BECAUSE WE EXPECT THE FIRST PROCESS TO BE CREATED TO BE A NO-PARENT SHELL
    active_terminal = 0;
    terminal_processes[0].active_process_PID = 0;
    terminal_processes[1].active_process_PID = -1;
    terminal_processes[2].active_process_PID = -1;
    int i;
    int j;
    // uint8_t * t1 = (uint8_t *)0xBA000;
    // uint8_t * t2 = (uint8_t *)0xBB000;
    // uint8_t * t3 = (uint8_t *)0xBC000;
    // for(i=0; i<4096; i++){
    //     *(uint8_t *)(t1+(i << 1) + 1) = ATTRIB;
    //     *(uint8_t *)(t2+(i << 1) + 1) = ATTRIB;
    //     *(uint8_t *)(t3+(i << 1) + 1) = ATTRIB;
        
    // }


    //the stuff below here initializes the video memory for each terminal
    for (i = 0; i < NUM_ROWS; i++) {
            int screen_x = 0;
            int screen_y = i;
            for (j = 0; j < NUM_COLS; j++) {

                *(uint8_t *)(video_mem1 + ((NUM_COLS * (screen_y) + screen_x) << 1) + 1) = ATTRIB;


                *(uint8_t *)(video_mem2 + ((NUM_COLS * (screen_y) + screen_x) << 1) + 1) = ATTRIB;

                *(uint8_t *)(video_mem3 + ((NUM_COLS * (screen_y) + screen_x) << 1) + 1) = ATTRIB;
                screen_x++;
                screen_x %= NUM_COLS;
            }
        }
}



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

void schedule() {
    // int temp = active_terminal;
    return;
}

