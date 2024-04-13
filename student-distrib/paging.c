#include "paging.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


#define VIDEO               0xB8000
#define KERNEL_START        0x01

#define     EIGHT_MB                            (1 << 23)// change back to 23// 4096 bytes * 8 bits per byte
#define     EIGHT_KB                            (1 << 13)



/*  This function initializes paging. It initializes the kernel memory and video memory
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

    //FOR THE SHELL PROGRAM
    page_directory[32].page_4mb.rw = 1;
    page_directory[32].page_4mb.us = 1; // supervisor mode for kernel obvi // FIX
    page_directory[32].page_4mb.p = 1;
    page_directory[32].page_4mb.ps = 1; // size is 1 for 4MB
    // page_directory[32].page_4mb.page_base_addr = SHELL_START;


    //mapping the vmem to the virtual addr 132 MB
    page_directory[33].page_4kb.pt_base_addr = ((uint32_t)vmem_page_table) >> 12; // put the addr in the page table
    page_directory[33].page_4kb.ps = 0; //size should be 4kb
    page_directory[33].page_4kb.us = 1; // user page for vmem
    page_directory[33].page_4kb.p = 1;

    for(i = 0; i < 1024; i++){
        vmem_page_table[i].p = 0;  //MARK ALL OTHER PAGES AS NONPRESENT
        vmem_page_table[i].rw = 1;
    }
    vmem_page_table[0].us = 1; // add a user page for vmem
    vmem_page_table[0].p = 1;

    loadPageDirectory((unsigned int *)page_directory);
    enableExtendedPageSize();
    enablePaging();



}


/* Description: This is an assembly helper function to load the page directory into control register 3.
* Inputs: inputs a page directory 
* Outputs: None
* Side Effects: paging directory stored in the control register
*/
void loadPageDirectory(unsigned int* pageDirectory){
asm volatile("                  \n\
            mov 8(%esp), %eax   \n\
            mov %eax, %cr3      \n\
                                ");
}

/* Description: This is an assembly helper function to load the extended page size bit into control register 4.
* Inputs: None
* Outputs: None
* Side Effects: 4MB pages will be able to be created
*/
void enableExtendedPageSize(){
    asm volatile("          \n\
    mov %cr4, %eax          \n\
    or $0x00000010, %eax    \n\
    mov %eax, %cr4          \n\
               ");
               return;
}
/* Description: This is an assembly helper function to load the activation bit into control register 0.
* Inputs: None
* Outputs: None
* Side Effects: Paging will be enabled
*/
void enablePaging(){
    asm volatile("          \n\
    mov %cr0, %eax          \n\
    or $0x80000000, %eax    \n\
    mov %eax, %cr0          \n\
               ");
    return;
}
