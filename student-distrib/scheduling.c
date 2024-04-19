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
    // og_x[0] = 1;
    // og_x[1] = 1;
    // og_x[2] = 1;
    // og_y[0] = 1;
    // og_y[1] = 1;
    // og_y[2] = 1;
    SHELLPROMPT_DELETE_FLAG[0] = 0;
    SHELLPROMPT_DELETE_FLAG[1] = 0;
    SHELLPROMPT_DELETE_FLAG[2] = 0;
    int i;
    int j;
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
    return;
}

/*  pit_handler()

Description: Handles PIT interrupts. We're able to do scheduling inside of the PIT, or alternatively we can make a separate scheduling function. Would prefer the latter option
*/

int shell_count = 0;
void pit_handler()  {
    register uint32_t ebp asm("ebp");
    uint32_t EBP_SAVE = ebp;
    
    if(shell_count < 3){
        terminal_processes[scheduled_terminal].EBP_SAVE = EBP_SAVE;
        
        uint8_t shell_var[6] = "shell";
        shell_count++;
        scheduled_terminal = shell_count -1;// on 0th will update 
        no_parent_shell_flag = 1;
        terminal_processes[scheduled_terminal].cursor_x = 7; //saving screenx/screeny
        terminal_processes[scheduled_terminal].cursor_y = 1;
        
        send_eoi(0);
        sys_execute(shell_var);
    }

    terminal_processes[scheduled_terminal].EBP_SAVE = EBP_SAVE;

    schedule();
    
    return; // hypotheticallu should never get here
}

void schedule() {
    //these cover whenever a few terminals haven't been activated yet

        if(TERMINAL1_SWITCH){
            first_page_table[0xB8].p_base_addr = 0xB8;
            vmem_page_table[0].p_base_addr = 0xB8; // set the current one to physical VMEM
            vmem_page_table[1].p_base_addr = 0xBB; // set the others to physical background memory
            vmem_page_table[2].p_base_addr = 0xBC; // set the others to physical background memory
            asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
            asm volatile("movl %ebx, %cr3");
            move_four_kb((uint8_t *) VIDEO, (uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB); // saving the current vmem
            terminal_processes[displayed_terminal].cursor_x = screen_x[displayed_terminal]; //saving screenx/screeny
            terminal_processes[displayed_terminal].cursor_y = screen_y[displayed_terminal];
            terminal_processes[displayed_terminal].togx = og_x[displayed_terminal]; //saving ogx/y
            terminal_processes[displayed_terminal].togy = og_y[displayed_terminal];
            //updating everything for terminal 0
            displayed_terminal = 0;
            move_four_kb((uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB, (uint8_t *) VIDEO) ; //moving the stored vmem into displayed vmem
            update_xy_display(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            update_cursor(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            og_x[displayed_terminal] = terminal_processes[displayed_terminal].togx;
            og_y[displayed_terminal] = terminal_processes[displayed_terminal].togy;
            TERMINAL1_SWITCH = 0;
            TERMINAL_READ_FLAG[0] = 1;          //NEEDED EVERY TIME WE SWITCH TERMINALS DUE TO HOW SHELL PROGRAM WORKS
            terminal_processes[0].togx = 7;
            send_eoi(0);
            return;
        }

        else if(TERMINAL2_SWITCH){
            first_page_table[0xB8].p_base_addr = 0xB8;
            vmem_page_table[0].p_base_addr = 0xBA; // set the others to the background
            vmem_page_table[1].p_base_addr = 0xB8; // set the current one to VMEM
            vmem_page_table[2].p_base_addr = 0xBC; // set the others to the background
            asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
            asm volatile("movl %ebx, %cr3");
            move_four_kb((uint8_t *) VIDEO, (uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB); // saving the current vmem
            terminal_processes[displayed_terminal].cursor_x = screen_x[displayed_terminal]; //saving screen x/y
            terminal_processes[displayed_terminal].cursor_y = screen_y[displayed_terminal];
            terminal_processes[displayed_terminal].togx = og_x[displayed_terminal]; // 
            terminal_processes[displayed_terminal].togy = og_y[displayed_terminal];
            displayed_terminal = 1;
            move_four_kb((uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB, (uint8_t *) VIDEO) ; //moving the stored vmem into displayed vmem
            update_xy_display(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            update_cursor(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            og_x[displayed_terminal] = terminal_processes[displayed_terminal].togx;
            og_y[displayed_terminal] = terminal_processes[displayed_terminal].togy;
            TERMINAL2_SWITCH = 0;             
            TERMINAL_READ_FLAG[1] = 1;          //NEEDED EVERY TIME WE SWITCH TERMINALS DUE TO HOW SHELL PROGRAM WORKS
            terminal_processes[1].togx = 7;
            send_eoi(0);
            return;
        }

        else if(TERMINAL3_SWITCH){
            first_page_table[0xB8].p_base_addr = 0xB8;
            vmem_page_table[0].p_base_addr = 0xBA; // set the current one to VMEM
            vmem_page_table[1].p_base_addr = 0xBB; // set the others to the background
            vmem_page_table[2].p_base_addr = 0xB8; // set the others to the background
            asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
            asm volatile("movl %ebx, %cr3");
            move_four_kb((uint8_t *) VIDEO, (uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB); // saving the current vmem
            terminal_processes[displayed_terminal].cursor_x = screen_x[displayed_terminal];
            terminal_processes[displayed_terminal].cursor_y = screen_y[displayed_terminal];
            terminal_processes[displayed_terminal].togx = og_x[displayed_terminal];
            terminal_processes[displayed_terminal].togy = og_y[displayed_terminal];
            displayed_terminal = 2;
            move_four_kb((uint8_t *) TERMINAL1_MEM + displayed_terminal*FOUR_KB, (uint8_t *) VIDEO) ; //moving the stored vmem into displayed vmem
            update_xy_display(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            update_cursor(terminal_processes[displayed_terminal].cursor_x, terminal_processes[displayed_terminal].cursor_y);
            og_x[displayed_terminal] = terminal_processes[displayed_terminal].togx;
            og_y[displayed_terminal] = terminal_processes[displayed_terminal].togy;
            TERMINAL3_SWITCH = 0;
            TERMINAL_READ_FLAG[2] = 1;          //NEEDED EVERY TIME WE SWITCH TERMINALS DUE TO HOW SHELL PROGRAM WORKS
            terminal_processes[2].togx = 7;
            send_eoi(0);
            return;
        }


    //all is saved, time to context switch!
    scheduled_terminal = (scheduled_terminal +1)% 3;
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




