
#include "file_sys_driver.h"
#include "lib.h"

//#include <stdio.h>
//#include <string.h>
//adding defines here for some file types --wyatt

#define REGULAR_FILE                    2
#define DIRECTORY_FILE                  1
#define RTC_FILE                        0
#define NUMBER_OF_FILE_CHARACTERS       32
#define FOUR_KB                   4096
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

    int32_t err_code = read_dentry_by_name(filename, opened_file);   //populates opened_file with data from the file
    
    //need to check if read_dentry_by_name returned an error code (can happen in case of filename being too long)
    //can also happen if no such file is found... i believe
    if  (err_code == -1)    {
        printf(" System attempted to call read_dentry_by_name, but call failed.\n");
        return -1;
    }

    //from TA - a directory file and a regular file may potentially have the same name. don't know why tho
    //(file types should never have the same name, but i suppose this may be an edge case tested during demos)
    if  (opened_file->file_type != REGULAR_FILE)    {
        printf(" System attempted to retrieve a regular file, but retrieved file type was different.\n");
        return -1;
    }

    //do something else here... not sure how to actually "open" a file. maybe include a parameter that
    //points to data in temp_dentry? will try doing that

    return 0;
}

int32_t file_read()    { //print data of a file
    return 0;
}


/* This is our write function, it isn't functional since our file system is read only right now 
*
*
*/
int32_t file_write()   {
    return -1; // DUNZO
}

int32_t file_close()   {
    return 0;
}



int32_t directory_open()    {
    return 0;
}

int32_t directory_close()   {
    return 0; // DUNZO
}

int32_t directory_read()    {
    return 0;
}

int32_t directory_write()   {
    return -1; // DUNZO
}
void print_number_of_inodes(){
	printf("This is the number of inodes: %d" , (int) booting_info_block->number_of_inodes);
}


//for read_data: we're given the inode, the offset from the start of data (beginning at the first data block, but potentially extending to other blocks),
//buf is what we're putting the data into, and length is the amount of bytes you want to write into the buffer


int32_t read_dentry_by_name (const uint8_t* fname, dentry_struct_t* dentry) {
    //i think the goal of this function is to search for a file in the boot block that corresponds to fname.
    //TA said we need to check for filename size. maximum char count for filename is 32 chars.
    //32 chars * 8 bits per char == 256 bits, or 32 bytes.

    //do strlen bullshit here
    uint32_t string_length = strlen((int8_t*) fname); // Doesn't allow standard libraries -- ask about later
    if(string_length > 32){
        return -1;
    }

    //after checking for filename size, we can loop through the list of dentry structs in the boot block to find the fname
    int i;
    int j;
    int file_found = 0;
    //int correct_string = 0;
    for (i = 0; i < 63; i++)    {
        for(j = 0; j < NUMBER_OF_FILE_CHARACTERS; j++){
            if (fname[j] == booting_info_block->dir_entries[i].file_name[j]){
                // printf("\n We made it into the name is correct if statement");
                //putc(fname[j]);
                if(fname[j] == NULL){
                    //printf("\n made it to the end of the string\n ");
                    file_found = 1; // we know we made it to the correct string
                }
            }
            else{
                break;
            }
            
         // Not sure if we need to worry about the case with multiple of the same file name
        }
        if(file_found){
            //printf("\n We made it into the numchar correct if statement \n");
            // for(j = 0; j < NUMBER_OF_FILE_CHARACTERS; j++){
            //     dentry->file_name[j] =  booting_info_block->dir_entries[i].file_name[j];
            // }
            //printf("The dentry index of this file is:  %d", i);
            strcpy(dentry->file_name, booting_info_block->dir_entries[i].file_name);
            dentry->file_type = booting_info_block->dir_entries[i].file_type;
            dentry->inode_number = booting_info_block->dir_entries[i].inode_number;
            // Not copying over reserved, hopefully we don't care
            return 0; // return 0 because we found the file;
        }


    }
    return -1; // File must not have been found
}

int32_t read_dentry_by_index(uint32_t index, dentry_struct_t * dentry){
    if(index < 0 || index > 62){
        return -1;
    } // We have a valid index

    strcpy(dentry->file_name, booting_info_block->dir_entries[index].file_name);
    dentry->file_type = booting_info_block->dir_entries[index].file_type;
    dentry->inode_number = booting_info_block->dir_entries[index].inode_number;
    return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t number_of_inodes = booting_info_block->number_of_inodes;
    uint32_t number_of_data_blocks = booting_info_block->number_of_data_blocks;
    printf("\n passes the bootblock setup variables \n");
    inode_struct_t * inode_address = (inode_struct_t*)(booting_info_block + 1 + inode);
    uint32_t actual_length_in_bytes = inode_address->length_in_bytes;
    printf("\n passes the initial setup variables \n");
    if(offset+length > actual_length_in_bytes){
        return -1;
    }
    // uint32_t total_number_of_data_blocks = (actual_length_in_bytes/FOUR_KB);
    int i;
    // int starting_data_block= offset/FOUR_KB; // ZERO INDEXED FROM THE LENGTH, ie if offset is zero, we at first block for the inode
    // int ending_data_block = (offset+length)/FOUR_KB; // ENDING DATA BLOCK,


    //local datablock index refers to the index of the datablock in the inode, if offset is 0, this would be 0, 
    //global datablock index gives you the index in terms of all the datablocks
    //full 4kb array index is the total array index of the structure starting with the 4kb boot block,
    //A super global index if you will
    for(i = offset; i < length+offset; i++){
        uint32_t local_datablock_index = i/FOUR_KB;
        uint32_t  global_datablock_index = inode_address->data_blocks[local_datablock_index];
        // buf[i] = global_datablock_index[i%FOUR_KB];
        if(global_datablock_index >= number_of_data_blocks){
            return -1; // unlikely case, but if so might as well see whats up
        }
        uint32_t full_4kb_array_index = 1 + number_of_inodes + global_datablock_index;
        uint32_t * datablock_addr = (uint32_t *)booting_info_block + full_4kb_array_index;
        buf[i- offset] = datablock_addr[i];
        
    }
    return 0;

}
