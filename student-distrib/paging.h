//header for paging code

/*
paging_init

Description: Initializes paging for the kernel.
    Sets up paging 
*/
#include "types.h"

typedef union paging_dir_t { // page directory entry
    uint32_t val[1];
    struct {
        uint16_t p : 1;
        uint16_t rw : 1;
        uint8_t  us : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t ps : 1;
        uint32_t glob_page : 1;
        uint32_t avail : 3;
        uint32_t pt_base_addr : 20;
    } __attribute__ ((packed));
} paging_dir_t;

typedef union paging_table_t { // page table entry
    uint32_t val[1];
    struct {
        uint16_t p : 1;
        uint16_t rw : 1;
        uint8_t  us : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pat : 1;
        uint32_t glob_page : 1;
        uint32_t avail : 3;
        uint32_t p_base_addr : 20;
    } __attribute__ ((packed));
} paging_table_t;

extern void paging_init();
void loadPageDirectory(unsigned int*);
void enableExtendedPageSize();
void enablePaging();
