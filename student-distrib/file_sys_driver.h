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

void file_system_init();

int32_t file_open(const uint8_t* filename, dentry_struct_t* opened_file); // populates a dentry

int32_t file_read(dentry_struct_t dentry, uint8_t * buf, uint32_t nbytes);

int32_t file_write();

int32_t file_close();

int32_t directory_open();

int32_t directory_close();

int32_t directory_read(); // 

int32_t directory_write();

int32_t read_dentry_by_name (const uint8_t* fname, dentry_struct_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_struct_t * dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
//Questions:

//do we need to implement syscalls for this?
    //I see we need to do read/write, but i thought syscalls was cp 3
    //it looks like the inode is variable length, how do I initialize that struct
    // Where are the data blocks going to be stored

    //MOD ADDRESS IS GOING TO CORRESPOND TO THE BOOT BLOCK

    //How does the boot block go at the 0th inode, but
    // inode struct as well??? 
    
    //boot block does not go at 0th inode. it is not part of the inodes. --wyatt

