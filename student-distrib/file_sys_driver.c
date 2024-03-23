
#include "file_sys_driver.h"
#include "lib.h"

//adding defines here for some file types --wyatt

#define REGULAR_FILE        2
#define DIRECTORY_FILE      1
#define RTC_FILE            0

//Note that this doesn't require paging to be working

boot_struct * booting_info_block;       //will be used to point to start of filesystem
                                            //the hierarchy of pointers is as follows:
                                            // booting_info_block = multiboot_info_t_struct->mods_addr->mod_start
                                            // turns out david already did all of that stuff so yippee

extern void get_bootblock_address(unsigned long addr){
    booting_info_block = (boot_struct*) addr;
}


void file_system_init() {
    // need to map the struct to the mod address. (already done with above function)
    // might not need to worry about this for checkpoint 2.
    // get_bootblock_address is already being called in kernel.c, so the bootblock is already initialized.
    // don't see anything else we need to do just yet
}


/*
file_open

Description: Opens a file in an existing filesystem. Searches for the file using read_dentry_by_name.
Points opened_file to the data contained in the identified file, assuming a matching file was found

Inputs: filename, opened_file

Outputs: Returns -1 in case of error, 0 otherwise

Side effects: Finds a file corresponding to inputted file name. "Opens" file by pointing opened_file to data contained
in the identified file.

*/
int32_t file_open(const uint8_t* filename, dentry_struct_t* opened_file)    {


    //dentry_struct_t* temp_dentry;
    int32_t err_code = read_dentry_by_name(filename, opened_file);   //populates temp_dentry with data from the file
    
    //need to check if read_dentry_by_name returned an error code (can happen in case of filename being too long)
    //can also happen if no such file is found... i believe
    if  (err_code == -1)    {
        printf("System attempted to call read_dentry_by_name, but call failed.\n");
        return -1;
    }

    //from TA - a directory file and a regular file may potentially have the same name. don't know why tho
    //(file types should never have the same name, but i suppose this may be an edge case tested during demos)
    if  (opened_file->file_type != REGULAR_FILE)    {
        printf("System attempted to retrieve a regular file, but retrieved file type was different.\n");
        return -1;
    }

    //do something else here... not sure how to actually "open" a file. maybe include a parameter that
    //points to data in temp_dentry? will try doing that
    //opened_file = temp_dentry;

    return 0;
}

int32_t file_read()    {
    return 0;
}


/* This is our write function, it isn't functional since our file system is read only right now 
*
*
*/
int32_t file_write()   {
    return -1;
}

int32_t file_close()   {
    return 0;
}



int32_t directory_open()    {
    return 0;
}

int32_t directory_close()   {
    return 0;
}

int32_t directory_read()    {
    return 0;
}

int32_t directory_write()   {
    return 0;
}
void print_number_of_inodes(){
	printf("This is the number of inodes: %d" , (int) booting_info_block->number_of_inodes);
}

//for read_data: we're given the inode, the offset from the start of data (beginning at the first data block, but potentially extending to other blocks),
//buf is what we're putting the data into, and length is the amount of bytes you want to write into the buffer


int32_t read_dentry_by_name (const uint8_t* fname, dentry_struct_t* dentry) {
    //do something really epic here
    return 0;
}
