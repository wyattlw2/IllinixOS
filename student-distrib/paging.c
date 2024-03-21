#include "paging.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


#define VIDEO       0xB8000

void paging_init()  {
    // PSE (page size extensions) flag bit 4 of CR4
    // PAE (physical address extension) flag bit 5 of CR4
    paging_dir_t page_directory[1024] __attribute__((aligned(4096)));
    paging_table_t first_page_table[1024] __attribute__((aligned(4096)));   // CONTAINS PAGE FOR VIDEO MEMORY AT INDEX 0

    int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i].rw = 1;
    }
    for(i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        //first_page_table[i] = (uint32_t)(i * 0x1000) | 3; // attributes: supervisor level, read/write, present. // initializing the offset value with read and write high as well

        first_page_table[i].p_base_addr = i;
        first_page_table[i].us = 1; //supervisor level
        first_page_table[i].rw = 1;
        first_page_table[i].p = 1;
    }
    page_directory[0].pt_base_addr = ((uint32_t)first_page_table);
    page_directory[0].ps = 0; //First page directory -- first page table is vmem
    page_directory[0].us = 1; // first page directory should be supervisor
    page_directory[0].p = 1;
    


    first_page_table[0].us = 1; // supervisor
    //first_page_table[0].p_base_addr = VIDEO;
    //first_page_table[0] 

    //page directory 1 is going to be 
    page_directory[1].ps = 1; // for the kernel memory, 4MB
    page_directory[1].us = 1; // supervisor only for kernel
    //page_directory[1].pt_base_addr = 0;
    page_directory[1].p = 1;



    

    //we will fill all 1024 entries in the table, mapping 4 megabytes
    
    loadPageDirectory(page_directory);
    enableExtendedPageSize();
    enablePaging();



};


void loadPageDirectory(unsigned int* pageDirectory){
asm volatile("                  \n\
            movl 8(%esp), %eax   \n\
            movl %eax, %cr3      \n\
                                ");
            return;
}

void enableExtendedPageSize(){
    asm volatile("          \n\
    movl %cr4, %eax          \n\
    orl $0x00000080, %eax    \n\
    movl %eax, %cr4          \n\
               ");
}

void enablePaging(){
    asm volatile("          \n\
    movl %cr0, %eax          \n\
    orl $0x80000000, %eax    \n\
    movl %eax, %cr0          \n\
               ");
    return;
}