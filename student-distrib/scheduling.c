#include "scheduling.h"
#include "syscalls.h"
#include "paging.h"
#include "terminal.h"
// #include "types.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_FREQUENCY 1193182

#define DESIRED_FREQUENCY 100

#define     EIGHT_MB                            (1 << 23)// change back to 23// 4096 bytes * 8 bits per byte
#define     EIGHT_KB                            (1 << 13)
#define     PID_OFFSET_TO_GET_PHYSICAL_ADDRESS      2
#define FOUR_KB                         4096

#define     VIDEO                               0xB8000
#define     TERMINAL1_MEM                       0xBA000

#define NUM_ROWS    25
#define NUM_COLS    80
#define ATTRIB      0xA


static char * video_mem1 = (char *) 0xBA000;
static char * video_mem2 = (char *) 0xBB000;
static char * video_mem3 = (char *) 0xBC000;

void terminal_init(){
    no_parent_shell_flag = 1; // WE DO THIS BECAUSE WE EXPECT THE FIRST PROCESS TO BE CREATED TO BE A NO-PARENT SHELL
    displayed_terminal = 0;
    scheduled_terminal = 0; // CP5
    terminal_processes[0].active_process_PID = 0;
    terminal_processes[1].active_process_PID = 1;
    terminal_processes[2].active_process_PID = 2;
    int i;
    int j;
    // first_page_table[0xB8].p_base_addr = (uint32_t) 0xBA;
    asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
    asm volatile("movl %ebx, %cr3");
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

/*  pit_handler()

Description: Handles PIT interrupts. We're able to do scheduling inside of the PIT, or alternatively we can make a separate scheduling function. Would prefer the latter option
*/

int shell_count = 0;
void pit_handler()  {
    register uint32_t ebp asm("ebp");
    uint32_t EBP_SAVE = ebp;
    

    // terminal_processes[scheduled_terminal].active_process_PID = ; SHOULD BE SAVED IN EXEC/HALT
    // printf("\n PIT INTERRUPT");
    if(shell_count < 3){
        terminal_processes[scheduled_terminal].EBP_SAVE = EBP_SAVE;
        
        uint8_t shell_var[6] = "shell";
        shell_count++;
        scheduled_terminal = shell_count -1;// on 0th will update 
        no_parent_shell_flag = 1;
        terminal_processes[scheduled_terminal].cursor_x = 7; //saving screenx/screeny
        terminal_processes[scheduled_terminal].cursor_y = 1;
        // printf("Shell created.\n");
        // terminal_processes[scheduled_terminal].cursor_x = screen_x; //saving screenx/screeny
        // terminal_processes[scheduled_terminal].cursor_y = screen_y;
        // terminal_processes[scheduled_terminal].togx = og_x; //saving ogx/y
        // terminal_processes[scheduled_terminal].togy = og_y;
        // if(scheduled_terminal != 0){
        //     first_page_table[0xB8].p_base_addr = (uint32_t)scheduled_terminal + 0xBA;
        
        // }else{
        //     first_page_table[0xB8].p_base_addr = (uint32_t)scheduled_terminal + 0xBA;
        // }
        // asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
        // asm volatile("movl %ebx, %cr3 ");
        
        send_eoi(0);
        sys_execute(shell_var);
    }

    terminal_processes[scheduled_terminal].EBP_SAVE = EBP_SAVE;
    // terminal_processes[scheduled_terminal].cursor_x = screen_x; //saving screenx/screeny
    // terminal_processes[scheduled_terminal].cursor_y = screen_y;
    // terminal_processes[scheduled_terminal].togx = og_x; //saving ogx/y
    // terminal_processes[scheduled_terminal].togy = og_y;


    //move_four_kb((uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB, (uint8_t *) VIDEO);
    schedule();
    // sti();
    return; // hypotheticallu should never get here
}

void schedule() {
    //these cover whenever a few terminals haven't been activated yet

    
    //all is saved, time to change context!
    scheduled_terminal = (scheduled_terminal +1)% 3;
    // map the video memory to the background buffer
    //if display_term == sch
    // if(displayed_terminal == scheduled_terminal){
    //     first_page_table[0xB8].p_base_addr = 0xB8;
    // }else{
    //     first_page_table[0xB8].p_base_addr = (uint32_t)scheduled_terminal + 0xBA;
    // }
    // screen_x = terminal_processes[scheduled_terminal].cursor_x;  
    // screen_y = terminal_processes[scheduled_terminal].cursor_y; 
    // og_x = terminal_processes[scheduled_terminal].togx;  
    // og_y = terminal_processes[scheduled_terminal].togy; 
    current_process_idx = terminal_processes[scheduled_terminal].active_process_PID;
    page_directory[32].page_4mb.page_base_addr = PCB_array[current_process_idx]->PID + PID_OFFSET_TO_GET_PHYSICAL_ADDRESS; //resetting the PID to be what it needs to be
    asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
    asm volatile("movl %ebx, %cr3");
    tss.esp0 = (EIGHT_MB - (PCB_array[current_process_idx]->PID)*EIGHT_KB) - 4; // Does this need to point to the start of the stack or the actual stack pointer itsel
    send_eoi(0);
    asm volatile ("movl %0, %%ebp;" : : "r" (terminal_processes[scheduled_terminal].EBP_SAVE));
    asm volatile ("movl %ebp, %esp");
    asm volatile ("pop %ebp");
    asm volatile("ret");


    return;
}




