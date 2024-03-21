#include "paging.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


#define VIDEO       0xB8000

void paging_init()  {
    // PSE (page size extensions) flag bit 4 of CR4
    // PAE (physical address extension) flag bit 5 of CR4
    uint32_t page_directory[1024] __attribute__((aligned(4096)));
    //set each entry to not present
    int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i] = 0x00000002;
    }
    // holds the physical address where we want to start mapping these pages to.
    // in this case, we want to map these pages to the very beginning of memory.
    unsigned int j;

    uint32_t first_page_table[1024] __attribute__((aligned(4096)));
    
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(j = 0; j < 1024; j++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_table[j] = (j * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }

    page_directory[0] = ((unsigned int)first_page_table) | 3;
    

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
}