#include "types.h"



void file_system_init();
void print_number_of_inodes();
void get_bootblock_address(uint32_t addr);


typedef struct inode_struct_t { // page directory entry
        uint32_t length_in_bytes;
        uint32_t data_blocks[1023];      //I'm CHOOSING to make this struct in right side up order because I think if we aren't messing with individual bits then it will be in order
} inode_struct_t;

typedef struct dentry_struct_t {
    char file_name[32]; //Allocating 32 bytes for file Name //might need to be uint32
    uint32_t  file_type;
    uint32_t  inode_number;
    uint8_t reserved[24];  //made this uint8 because 1 byte, but idk

} dentry_struct_t;

typedef struct boot_struct {
    uint32_t number_of_dir_entries; //Allocating 32 bytes for file Name //might need to be uint32
    uint32_t number_of_inodes;
    uint32_t number_of_data_blocks;
    uint8_t reserved[52];  //made this char, but idk
    dentry_struct_t dir_entries[63];

} boot_struct;
typedef struct data_block_struct_t {
    uint8_t data[4096];
} data_block_struct_t;



typedef struct function_pointers_t {
    int32_t (*open)(uint8_t * filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void * buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void * buf, int32_t nbytes);
} function_pointers_t;


typedef struct fd_t {
    function_pointers_t file_operations_table_pointer;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
    //int32T (*open)(int32 fd, buf ,)
} fd_t;
// static  rtc = {rtc_open, rtc_close, };
typedef struct file_descriptor_array_t {
    fd_t fd_entry[8];
} file_descriptor_array_t;

//NEED TO GET MORE INFO ON THIS -- specifically where to initialize it
typedef struct process_control_block_t { // THE PCB IS GOING TO BE STORED AT THE TOP OF A GIVEN KERNEL STACK
    int32_t PID;
    int32_t parent_PID; 
    int32_t EBP;
    int32_t ESP;
    int32_t EIP;
    file_descriptor_array_t fdesc_array; // probably will also need some mechanism to see if stack overflow occurs
    
    //signal information
    //possibly all the registers
    //task state information
    //stack pointer for a given process
} process_control_block_t;

int32_t processes_active[6]; // need to figure out more about initializing this POSSIBLY SET IT ALL TO ZERO
process_control_block_t* PCB_array[6];  //MUST be an array of pointers right now --W
int32_t current_process_idx;



void file_system_init();

int32_t file_open(const uint8_t* filename); // populates a dentry

int32_t file_read(int32_t fd, uint8_t * buf, uint32_t nbytes);

int32_t file_write();

int32_t file_close(int32_t fd);

int32_t directory_open(const uint8_t* filename);

int32_t directory_close(int32_t fd);

int32_t directory_read(int32_t fd, uint8_t * buf, uint32_t nbytes); // 

int32_t directory_write();

int32_t read_dentry_by_name (const uint8_t* fname, dentry_struct_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_struct_t * dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

void see_all_files_helper();
//Questions:

//do we need to implement syscalls for this?
    //I see we need to do read/write, but i thought syscalls was cp 3
    //it looks like the inode is variable length, how do I initialize that struct
    // Where are the data blocks going to be stored

    //MOD ADDRESS IS GOING TO CORRESPOND TO THE BOOT BLOCK

    //How does the boot block go at the 0th inode, but
    // inode struct as well??? 
    
    //boot block does not go at 0th inode. it is not part of the inodes. --wyatt

