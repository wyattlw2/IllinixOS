//header for paging code

/*
paging_init

Description: Initializes paging for the kernel.
    Sets up paging 
*/
#include "types.h"
/*  The structs below are used for the initialization of Paging.
*   There is two for Page Directories -- one for 4kb and one for 4mb and one for Page tables --
*   
*/

//4kb Page Directory Struct
typedef struct  __attribute__ ((packed)) paging_dir_4kb_t { // page directory entry
        uint32_t p                  : 1;
        uint32_t rw                 : 1;
        uint32_t us                 : 1;
        uint32_t pwt                : 1;
        uint32_t pcd                : 1;
        uint32_t accessed           : 1;
        uint32_t reserved           : 1;
        uint32_t ps                 : 1;
        uint32_t glob_page          : 1;
        uint32_t avail              : 3;
        uint32_t pt_base_addr       : 20;
} paging_dir_4kb_t;

// 4MB Page Directory Struct
typedef struct __attribute__ ((packed)) paging_dir_4mb_t { // page directory entry
        uint32_t p              : 1;
        uint32_t rw             : 1;
        uint32_t us             : 1;
        uint32_t pwt            : 1;
        uint32_t pcd            : 1;
        uint32_t accessed       : 1;
        uint32_t dirty          : 1;
        uint32_t ps             : 1;
        uint32_t glob_page      : 1;
        uint32_t avail          : 3;
        uint32_t pat            : 1;
        uint32_t reserved       : 9;
        uint32_t page_base_addr : 10;
} paging_dir_4mb_t ;

//Page Directory Struct
typedef union page_directory_entry {
    paging_dir_4kb_t page_4kb;
    paging_dir_4mb_t page_4mb;
} paging_dir_entry_t;

//Paging Table Struct
typedef struct  __attribute__ ((packed)) paging_table_t { // page table entry
     
        uint32_t p : 1;
        uint32_t rw : 1;
        uint32_t us : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pat : 1;
        uint32_t glob_page : 1;
        uint32_t avail : 3;
        uint32_t p_base_addr : 20;
    
} paging_table_t ;

extern void paging_init();
extern void paging_init_osdev();
void loadPageDirectory(unsigned int * pageDirectory);
void enableExtendedPageSize();
void enablePaging();

// extern void loadPageDirectoryOsdev(unsigned int *);
// extern void enablePagingOsdev();
