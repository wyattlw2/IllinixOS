#include "idt.h"
#include "file_sys_driver.h"
#include "paging.h"
#include "x86_desc.h"   //need to include so we can modify ESP0 field of TSS -- wyatt
#include "terminal.h"
#include "rtc.h"

#define KERNEL_CS   0x0010
#define KERNEL_DS   0x0018
#define USER_CS     0x0023
#define USER_DS     0x002B
#define KERNEL_TSS  0x0030
#define KERNEL_LDT  0x0038

#define FD_START                2
#define FD_END                  8
#define REGULAR_FILE                    2
#define DIRECTORY_FILE                  1
#define RTC_FILE                        0

#define     USER_PROG_0                         0x02
#define     USER_PROG_1                         0x03
#define     USER_PROG_2                         0x04
#define     USER_PROG_3                         0x05
#define     USER_PROG_4                         0x06
#define     USER_PROG_5                         0x07
#define     MAX_NUM_PROCESSES                   6
#define     VIRTUAL_USER_ADDR_WITH_OFFSET       0x08048000
#define     PAGE_DIR_INDEX_FOR_USER_PROG        32

#define     MAGIC_NUMBER_BYTE0                  0x7F
#define     MAGIC_NUMBER_BYTE1                  0x45
#define     MAGIC_NUMBER_BYTE2                  0x4C
#define     MAGIC_NUMBER_BYTE3                  0x46

#define     EIGHT_MB                            (1 << 23)// change back to 23// 4096 bytes * 8 bits per byte
#define     EIGHT_KB                            (1 << 13)
#define     PID_OFFSET_TO_GET_PHYSICAL_ADDRESS      2

#define     FAILURE         -1

int32_t num_active_processes = 0;   // cannot be decremented below 1 once it reaches that value. used in sys_halt() to determine
                                    // if halting the process will result in 0 running programs.

int32_t prev_PID = 70;  //initialized to 70 because we need to signify that the first process has no valid parent PID


static int32_t *rtc_functions[4] = {(int32_t*)rtc_open, (int32_t*)rtc_close, (int32_t*)rtc_read, (int32_t*)rtc_write};
static int32_t *directory_functions[4] = {(int32_t*)directory_open, (int32_t*)directory_close, (int32_t*)directory_read, (int32_t*)directory_write};
static int32_t *file_functions[4] = {(int32_t*)file_open, (int32_t*)file_close, (int32_t*)file_read, (int32_t*)file_write};


// Below are a sequence of system call handlers that correspond to a subset
// of Linux's system calls. These are necessary for user-level programs like
// execute, sigtest, shell, and fish to work. Each of those programs will be calling
// some of these handlers via interrupt 0x80    -- Wyatt






void sys_halt(uint8_t status) {
    
    if(num_active_processes == 1)   {
        printf("\n Can't Close Shell!! \n");
        int8_t var[32] = {"shell"};
        //restarting shell sequence
            
        prev_PID = 70;                  //signify that the process has no parent process

        num_active_processes--;         //must do this here because we increment this value in execute

        processes_active[current_process_idx] = 0;  //signify that the currently executing program needs to be re-executed in the same spot!
                                                    //this is hardcoded to call shell.exe for now, but why would you want to call anything else?
        asm volatile (
            "movl %0, %%ebx;"   // Move the address of var into register ebx
            :                   // Output operand list is empty
            : "r" (var)         // Input operand list, specifying that var is an input
        );
        asm volatile (
            "movl $2, %eax"     // Set syscall number to 2 (sys_exec)
        );
        asm volatile (
            "int $0x80"         // Execute syscall
        );
        return;
        // return -5000746; //cannot halt the program if there will be zero running programs
    }

    processes_active[current_process_idx] = 0;  //signify that the currently executing program needs to be re-executed in the same spot!

    page_directory[32].page_4mb.page_base_addr = PCB_array[current_process_idx]->parent_PID + PID_OFFSET_TO_GET_PHYSICAL_ADDRESS; //resetting the PID to be what it needs to be
    
    asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
    asm volatile("movl %ebx, %cr3");
    
    tss.esp0 = (EIGHT_MB - (PCB_array[current_process_idx]->parent_PID)*EIGHT_KB) - 4; // Does this need to point to the start of the stack or the actual stack pointer itself

     //updating current process index to be the parent's PID
    prev_PID = PCB_array[current_process_idx]->parent_PID;  //update previous process index to be the parent's parent_PID
    num_active_processes--;

    int32_t treg = PCB_array[current_process_idx]->EBP;    //old value of ebp that we saved during sys_execute()
    current_process_idx = PCB_array[current_process_idx]->parent_PID;
    // printf("\n EBP we are restoring INSIDE HALT IS: %d", treg);

    int32_t check_for_exception;
    // asm volatile(
    //     "movl %%eax, %0;" 
    //     : "=r" (check_for_exception) 
    //     : 
    //     : "eax" 
    // );
    // if(check_for_exception == 256)  {
    //     asm volatile ("movl %0, %%ebp;" : : "r" (treg));
    //     asm volatile ("movl %ebp, %esp");
    //     asm volatile ("pop %ebp");
    //     asm volatile("ret");
    // }

    uint8_t err_code;
    asm volatile(
        "movb %%bl, %0;" 
        : "=r" (err_code) 
        : 
        : "ebx" 
    );

    asm volatile ("movl %0, %%ebp;" : : "r" (treg));
    asm volatile ("movl %ebp, %esp");
    asm volatile ("pop %ebp");

    //SETTING THE RETURN VALUE -dvt
    asm volatile("xorl %eax, %eax");
    asm volatile (
            "movb %0, %%al;"   // Move the address of var into register ebx
            :                   // Output operand list is empty
            : "r" (err_code)         // Input operand list, specifying that var is an input
        );
    asm volatile("ret");
}



//TA (Jeremy I think) specified that for CP3, we do not need to worry about having multiple shells running at once - a shell can open a
//shell, but for now we do not need to worry about more than one program executing at a time. --W
int32_t sys_execute(uint8_t * command) {
    register uint32_t ebp asm("ebp");
    register uint32_t esp asm("esp");
    uint32_t ebp_save = ebp;
    uint32_t esp_save = esp;

    int32_t retval = 256;      // sys_execute needs to return 256 in the case of an exception
    dentry_struct_t exec_dentry;  
    int32_t found_file = read_dentry_by_name(command, &exec_dentry);
    // uint8_t buffer[60000];
    
    
    if(found_file == -1){
        printf("\n The inputted file was invalid. \n");
        return -1; // might need more checks for this lmao
    }
    int i;

    //process_activating is going to be the process ID NUMBER
    // process_control_block_t * PCB;  //going to turn into a global variable for now -- W

    process_control_block_t * PCB;
    int PID = 500; // ridiculous value, if it is still 500, we didn't find a process and we return out
        for(i = 0; i< MAX_NUM_PROCESSES; i++){ // start at process 
            if(processes_active[i] == 0){ // this process is empty and thus we assign the virtual addr
                num_active_processes++;
                processes_active[i] = 1;
                PID = i; // ASSIGNING PROCESS ID NUMBER
                PCB = (process_control_block_t *) (EIGHT_MB - (PID+1)*EIGHT_KB); // ASSIGNS THE ADDRESS OF THE PCB based on what process it is
                PCB_array[i] = PCB;
                PCB->parent_PID = prev_PID; // specifies if the current process is a child of another process.
                                            // if it is, set it equal to the PID of the parent process.
                                            // otherwise, set the PID to an absolutely crazy value to signify that it
                                            // is not a child process    
                // printf("(Inside Exec) Prev PID: %d\n\n", prev_PID);

                switch(PID){
                    case(0):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_0;
                        break;
                    case(1):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_1;
                        break;
                    case(2):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_2;
                        break;
                    case(3):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_3;
                        break;
                    case(4):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_4;
                        break;
                    case(5):
                        page_directory[32].page_4mb.page_base_addr = USER_PROG_5;
                        break;
                }
                
                //FLUSH TLB
                asm volatile("movl %cr3, %ebx"); //gaslighting the system, thinking that the page directory has changed -- FLUSHES TLB
                asm volatile("movl %ebx, %cr3");

                break;
            } 

        }
        if(PID == 500){
            printf("\n The Maximum Number of Processes are being used \n");
            return -1; // MIGHT NEED MORE THAN A RETURN HERE, WORRY ABOUT IT LATER
        }

        //TIME TO LOAD THE USER LEVEL PROGRAM
        
        //Before loading file data into virtual address space, we need to check the first four bytes of the file
        //The first four bytes must correspond to ELF
        uint8_t mag_num_buf[30]; // header is actually 40 bytes
        int32_t mag_num_check = read_data((&exec_dentry)->inode_number, 0, mag_num_buf , 30);
        

        int32_t EIP_save = ((uint32_t*)mag_num_buf)[6]; // Extracting the EIP from the string
        // printf("\n EIP TRY 2: %d", EIP_try2);
        
        // int32_t EIP_save = eip_ll << 24 | eip_l << 16 | eip_r << 8 | eip_rr;
        if(mag_num_check == -1){
            printf(" \n Something screwed up inside the excecutable file, you should really check that out \n");
            return -1;
        }
        if((mag_num_buf[0] != MAGIC_NUMBER_BYTE0 )||( mag_num_buf[1] != MAGIC_NUMBER_BYTE1 )||( mag_num_buf[2] != MAGIC_NUMBER_BYTE2 )||( mag_num_buf[3] != MAGIC_NUMBER_BYTE3)){
            printf(" \n Something messed up inside the excecutable file, you should really check that out -- Magic Number Test Failed \n");
            return -1;
        }


        uint8_t * user_start = (uint8_t *)VIRTUAL_USER_ADDR_WITH_OFFSET;
        int32_t status = read_data((&exec_dentry)->inode_number, 0, user_start, 0x400000);
        if(status == -1){
            printf("\n Something went wrong when copying the data over \n");
        }
                                                                                                // ONE GLOBAL TSS
                                                                                                //TSS Has information about how to get back to kernel, ie HALT?
                                                                                                //when we hit up checkpoint 5 is this messed up because 
        
        PCB->PID = PID;
        PCB->EBP = ebp_save;
        PCB->ESP = esp_save;
        prev_PID = PID;     //Have to save the current PID as the last PID
        current_process_idx = PID;
        PCB->fdesc_array.fd_entry[0].file_operations_table_pointer.read = t_read; //setting std in
        PCB->fdesc_array.fd_entry[1].file_operations_table_pointer.write = t_write; // setting std out

        tss.esp0 = (EIGHT_MB - (PID)*EIGHT_KB) - 4; // updating the esp0

        asm volatile("pushl $0x002B"); // pushing User DS
        asm volatile("pushl $0x083ffffc"); //This Userspace stack pointer always starts here
        asm volatile("pushfl"); // this could be off -- pushing flags
        asm volatile("pushl $0x0023");  //pushing the USER CS
        asm volatile("pushl %0" : : "r" (EIP_save) : "memory");  // push EIP that was stored in the executable file
       
        asm volatile("iret ");

        
    //Will Never Return here
    return retval;
}


int32_t sys_read(int32_t fd, void * buf, int32_t nbytes) {

    return (* PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.read)(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, void * buf, int32_t nbytes) {

    int32_t retval = (* PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.write)(fd, buf, nbytes);
    if(retval == -1){
        return -1;
    }
    return 0;
}



int32_t sys_open(int8_t * filename) {
    // printf("\n made it to sys open \n ");

    int i;
    int fd_index_to_open;
    for(i = FD_START; i < FD_END; i++){ // find an open fd
        if(PCB_array[current_process_idx]->fdesc_array.fd_entry[i].flags == 0){ // if it's not occupied, set the file index we are going to use
            fd_index_to_open = i;
            break;
        }
    }

    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_position = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].inode = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].flags = 1; // declaring a fd 

    dentry_struct_t file_to_open;
    int32_t retval = read_dentry_by_name((uint8_t *)filename, &file_to_open);
    if(retval == FAILURE){
        printf("\n Sys Open Failed \n");
        return FAILURE;
    }
    if(file_to_open.file_type == RTC_FILE){
        //It is an RTC file
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open = (void *)rtc_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close = (void *)rtc_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)rtc_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)rtc_functions[3];
        int32_t rtc_open_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(rtc_open_stat == FAILURE){
            printf("\n Sys Open Failed -- RTC_Open \n");
            return FAILURE;
        }
    }else if(file_to_open.file_type == DIRECTORY_FILE){
        //it is a directory
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open =  (void *)directory_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close =  (void *)directory_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)directory_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)directory_functions[3];
        int32_t dopen_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(dopen_stat == FAILURE){
            printf("\n Sys Open Failed -- D_Open \n");
            return FAILURE;
        }
        
        //printf(" \n\n DIRECTORY Janky directory jumptable called here!\n\n");
        //asm volatile("jmp directory_jumptable");
    }else{
        //It is a file
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open =  (void *)file_functions[0];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.close =  (void *)file_functions[1];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.read =  (void *)file_functions[2];
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.write =  (void *)file_functions[3];
        int32_t fopen_stat = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].file_operations_table_pointer.open)((uint8_t*) filename);
        if(fopen_stat == FAILURE){
            printf("\n Sys Open Failed -- F_Open \n");
            return FAILURE;
        }
        PCB_array[current_process_idx]->fdesc_array.fd_entry[fd_index_to_open].inode = file_to_open.inode_number;     // setting inode number
    }
    return fd_index_to_open;
}

int32_t sys_close(int32_t fd) {
    if(PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].flags == 0){
        printf("\n Attempted to Close Something that was not open in the first place \n");
        return FAILURE;
    } 
    int32_t retval = (*PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.close)(fd);
    if(retval == FAILURE){
        return FAILURE;
    }
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.open = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.close = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.read = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_operations_table_pointer.write = (void *)0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].inode = 0;
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].flags = 0;
    

    printf("SYSCALL *CLOSE* CALLED (SHOULD CORRESPOND TO SYSCALL 6)\n\n");
    return 0;
}

int32_t sys_getargs(uint8_t * buf, int32_t nbytes) {
    printf("SYSCALL *GETARGS* CALLED (SHOULD CORRESPOND TO SYSCALL 7)\n\n");
    return 0;
}

int32_t sys_vidmap(uint8_t ** screen_start) {
    printf("SYSCALL *VIDMAP* CALLED (SHOULD CORRESPOND TO SYSCALL 8)\n\n");
    return 0;
}

int32_t sys_set_handler(int32_t signum, void * handler_address) {
    printf("SYSCALL *SET_HANDLER* CALLED (SHOULD CORRESPOND TO SYSCALL 9)\n\n");
    return 0;
}

int32_t sys_sigreturn() {
    //no args i think? --dvt
    printf("SYSCALL *SIGRETURN* CALLED (SHOULD CORRESPOND TO SYSCALL 10)\n\n");
    return 0;
}
int32_t sys_error(){
    printf("\n Something went wrong, It is possible, the wrong system call index was provided. \n");
    return -1;
}
