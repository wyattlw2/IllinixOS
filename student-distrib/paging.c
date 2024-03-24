#include "paging.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


#define VIDEO               0xB8000
#define KERNEL_START        0x01

paging_dir_entry_t page_directory[1024] __attribute__((aligned(4096)));
paging_table_t first_page_table[1024] __attribute__((aligned(4096)));   // CONTAINS PAGE FOR VIDEO MEMORY AT INDEX 0
/*  THis function Initializes paging. it's really cool you should check it out. It initializes the kernel memory and video memory
*   Inputs: None
    Outputs: none
*   Side Effects:
*   It also modifies CR0, CR4, and CR3 to initialize paging and allow for extended
*/
void paging_init()  {
    // PSE (page size extensions) flag bit 4 of CR4
    // PAE (physical address extension) flag bit 5 of CR4
    

    int i;
    for(i = 0; i < 1024; i++)
    {
        page_directory[i].page_4kb.rw = 1; 
        page_directory[i].page_4kb.p = 0; 
    }
    page_directory[0].page_4kb.pt_base_addr = ((uint32_t)first_page_table) >> 12; // Might need to bit shift this or something
    page_directory[0].page_4kb.ps = 0; //First page directory -- first page table is vmem
    page_directory[0].page_4kb.us = 0; // first page directory should be supervisor
    page_directory[0].page_4kb.p = 1;

    for(i = 0; i < 1024; i++){
        first_page_table[i].p = 0;  //MARK ALL OTHER PAGES AS NONPRESENT
        first_page_table[i].rw = 1;
    }
    first_page_table[0xB8].us = 0; //NNEEEDS TO BE THE B8 INDEX
    first_page_table[0xB8].p = 1;
    first_page_table[0xB8].p_base_addr = VIDEO >> 12;


    //KERNEL DIRECTORY SETUP
    page_directory[1].page_4mb.rw = 1;
    page_directory[1].page_4mb.us = 0; // supervisor mode for kernel obvi // FIX
    page_directory[1].page_4mb.p = 1;
    page_directory[1].page_4mb.ps = 1; // size is 1 for 4MB
    page_directory[1].page_4mb.page_base_addr = KERNEL_START;

    loadPageDirectory((unsigned int *)page_directory);
    enableExtendedPageSize();
    enablePaging();



}



uint32_t page_directoryOsdev[1024] __attribute__((aligned(4096)));
uint32_t first_page_tableOsdev[1024] __attribute__((aligned(4096)));
void paging_init_osdev(){

    
    //set each entry to not present
    int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directoryOsdev[i] = 0x00000002;
    }
    

        //unsigned int i;
    
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_tableOsdev[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }
    page_directoryOsdev[0] = ((unsigned int)first_page_tableOsdev) | 3;

    loadPageDirectory(page_directoryOsdev);
    //enableExtendedPageSize();
    enablePaging();
}


void loadPageDirectory(unsigned int* pageDirectory){
asm volatile("                  \n\
            mov 8(%esp), %eax   \n\
            mov %eax, %cr3      \n\
                                ");
}

void enableExtendedPageSize(){
    asm volatile("          \n\
    mov %cr4, %eax          \n\
    or $0x00000010, %eax    \n\
    mov %eax, %cr4          \n\
               ");
               return;
}

void enablePaging(){
    asm volatile("          \n\
    mov %cr0, %eax          \n\
    or $0x80000000, %eax    \n\
    mov %eax, %cr0          \n\
               ");
    return;
}
//void  enablePagingOsdev(){
    //asm volatile("
    // .text
    // .globl loadPageDirectoryOsdev
    // loadPageDirectoryOsdev:
    // push %ebp
    // mov %esp, %ebp
    // mov 8(%esp), %eax
    // mov %eax, %cr3
    // mov %ebp, %esp
    // pop %ebp
    // ret
               // ")


    // .text
    // .globl enablePagingOsdev
    // enablePagingOsdev:
    // push %ebp
    // mov %esp, %ebp
    // mov %cr0, %eax
    // or $0x80000000, %eax
    // mov %eax, %cr0
    // mov %ebp, %esp
    // pop %ebp
    // ret
//}

