
#include "file_sys_driver.h"
#include "lib.h"

//#include <stdio.h>
//#include <string.h>
//adding defines here for some file types --wyatt

#define REGULAR_FILE                    2
#define DIRECTORY_FILE                  1
#define RTC_FILE                        0
#define NUMBER_OF_FILE_CHARACTERS       32
#define FOUR_KB                         4096
//Note that this doesn't require paging to be working

boot_struct * booting_info_block;       //will be used to point to start of filesystem
                                            //the hierarchy of pointers is as follows:
                                            // booting_info_block = multiboot_info_t_struct->mods_addr->mod_start
                                            // turns out david already did all of that stuff so yippee

extern void get_bootblock_address(uint32_t addr){
    booting_info_block = (boot_struct*) addr;
}


/*
file_open
Description: Opens a file in an existing filesystem. Searches for the file using read_dentry_by_name.
    Points opened_file to the data contained in the identified file, assuming a matching file was found
Inputs: filename, opened_file
Outputs: Returns -1 in case of error, 0 otherwise
Side Effects: Finds a file corresponding to inputted file name. "Opens" file by pointing opened_file to data contained
    in the identified file.
*/
int32_t file_open(const uint8_t* filename)    {
    dentry_struct_t opened_file;
    int32_t err_code = read_dentry_by_name(filename, &opened_file);   //populates opened_file with data from the file
    
    //need to check if read_dentry_by_name returned an error code (can happen in case of filename being too long)
    //can also happen if no such file is found... i believe
    if  (err_code == -1)    {
        printf("\n System attempted to call read_dentry_by_name, but call failed.\n");
        return -1;
    }

    //from TA - a directory file and a regular file may potentially have the same name. don't know why tho
    //(file types should never have the same name, but i suppose this may be an edge case tested during demos)
    if  (opened_file.file_type != REGULAR_FILE)    {
        printf("\n System attempted to retrieve a regular file, but retrieved file type was different.\n");
        return -1;
    }

    //do something else here... not sure how to actually "open" a file. maybe include a parameter that
    //points to data in temp_dentry? will try doing that

    return 0;
}


/*
file_read
Description: Takes in a dentry, buf and nbytes and outputs the requested number of bytes into that buffer
Inputs: directory entry, buffer, nbytes
Outputs: Returns -1 in case of error, 0 otherwise
Side Effects: Data is populated into the buffer
*/


//NEEDS FIX, MAKE SURE THAT THIS RETURNS NBYTES READ
//AND ALSO NEEDS TO ALLOW READING GREATER THAN THE LENGTH OF THE FILE, but JUST STOP
int32_t file_read(int32_t fd, uint8_t * buf, uint32_t nbytes)    { //print data of a file
    int32_t inode_num = PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].inode;
    int32_t offset = PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position;
    
    if(inode_num < 0 || inode_num > 64){ // max number of inodes is 64
        printf("\n Invalid Directory Entry, Please try again with a vaild Directory Entry \n");
        return -1;
    }
    if(nbytes <=0){
        printf("\n Invalid Number of Bytes, please try again with the correct number of bytes \n");
        return -1;
    }

    int retval = read_data(inode_num, offset, buf, nbytes); // all legwork done in here
    if(retval == -1){
        printf("\n read_data was attempted, but it failed, please try again \n");
        return -1;
    }

    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position +=retval;
    return retval;
}


/*
file_write
Description: This function is non functional because our file system is read only
Inputs: None
Outputs: None
Side Effects: None
*/
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes)  {
    return -1; // DUNZO
}
/*
file_close
Description: This function effectively overwrites the dentry provided so that the file cannot be read again unless file open is called again
Inputs: Dentry of an opened file
Outputs: success (0) or failure (-1)
Side Effects: None
*/
int32_t file_close(int32_t fd)   {
    if(fd >=8 || fd < 0){
        printf("\n Unable to close the directory \n");
        return -1;
    }
    return 0; // DUNZO
}


/*
directory_open
Description: This function takes a file name and populates in the opened directory information into the dentry provided 
Inputs: file name and empty directory entry
Outputs: success (0) or failure (-1) 
Side Effects: None
*/
int32_t directory_open(const uint8_t* filename){
    dentry_struct_t opened_file;
    int32_t err_code = read_dentry_by_name(filename, &opened_file);   //populates opened_file with data from the file
    
    //need to check if read_dentry_by_name returned an error code (can happen in case of filename being too long)
    //can also happen if no such file is found... i believe
    if  (err_code == -1)    {
        printf("\n System attempted to call read_dentry_by_name for directory open, but call failed.\n");
        return -1;
    }

    //from TA - a directory file and a regular file may potentially have the same name. don't know why tho
    //(file types should never have the same name, but i suppose this may be an edge case tested during demos)
    if  ((&opened_file)->file_type != DIRECTORY_FILE)    {
        printf("\n System attempted to open a Directory, but retrieved file type was different.\n");
        return -1;
    }

    //do something else here... not sure how to actually "open" a file. maybe include a parameter that
    //points to data in temp_dentry? will try doing that

    return 0;
}

/*
directory_close
Description: This function takes an open dentry and 
Inputs: Dentry of an opened directory
Outputs: success (0) or failure (-1)
Side Effects: None
*/
int32_t directory_close(int32_t fd)   { // PRETTY MUCH A CARBON COPY OF file_close
    // int i;
    if(fd >=8 || fd < 0){
        printf("\n Unable to close the directory \n");
        return -1;
    }
    return 0; // DUNZO
}

/*
directory_read
Description: This function takes an open dentry, buffer, and num bytes and returns the directory name based on the information provided
Inputs: Dentry of an opened directory, buffer, and nbytes
Outputs: success (0) or failure (-1)
Side Effects: None
*/
int32_t directory_read(int32_t fd, uint8_t * buf, uint32_t nbytes)    { // if num bytes is too big then fail
    
    int index = PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position;
    dentry_struct_t dentry;
    int retval = read_dentry_by_index(index, &dentry);
    if(retval == -1){
        return -1;
    }
    int i;
    // int count = 0;
    for(i=0; i< nbytes; i++){
        if(dentry.file_name[i] == '\0'){
            break;
        }
        buf[i] = dentry.file_name[i];
    }
    PCB_array[current_process_idx]->fdesc_array.fd_entry[fd].file_position++;
    return i;
}

//Assumes 32
// int32_t directory_read_helper(uint32_t offset, uint8_t* buf, uint32_t length){
    // return 0;
    // int i;
    // if(offset+length)
    // for(i=offset; i<length; i++){
        // uint32_t cur_dir_name_length = strlen(booting_info_block->dir_entries[0].file_name)
        // buf[i-offset] = booting_info_block->dir_entries[]
    // }
    // int end = 0;
    // int index = offset;
    // while(end == 0){
    //     int32_t current_file_index;
    //     dentry_struct_t dentry;
    //     for(i=0;i<index; i++){
    //         if()
    //         read_dentry_by_index( , &dentry)
    //     }

    //     buf[index] = 
    //     index++;
    //     if(index-offset >= length){
    //         end = 1;
    //     }
    // }
// }
/*
directory_write
Description: This function is non functional because our file system is read only
Inputs: None
Outputs: None
Side Effects: None
*/
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes)   {
    return -1; // DUNZO
}

//helper function used to test if file system is reading properly, ignore,
void print_number_of_inodes(){
	printf("This is the number of inodes: %d" , (int) booting_info_block->number_of_inodes);
}


//for read_data: we're given the inode, the offset from the start of data (beginning at the first data block, but potentially extending to other blocks),
//buf is what we're putting the data into, and length is the amount of bytes you want to write into the buffer

/*
read_dentry_by_name
Description: This function takes in a file name and populates a dentry based on the name created
Inputs: file name, and empty dentry
Outputs: None
Side Effects: the dentry is populated with the correct dentry based on the file name inputted 
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_struct_t* dentry) {
    //i think the goal of this function is to search for a file in the boot block that corresponds to fname.
    //TA said we need to check for filename size. maximum char count for filename is 32 chars.
    //32 chars * 8 bits per char == 256 bits, or 32 bytes.

    //do strlen bullshit here
    uint32_t string_length = strlen((int8_t*) fname); // Doesn't allow standard libraries -- ask about later
    if(string_length > NUMBER_OF_FILE_CHARACTERS){
        printf("read_dentry_by_name: STRING TOO BIG\n");
        return -1;
    }
    else if (string_length == 0)    {
        printf("read_dentry_by_name: STRING IS EMPTY\n");
        return -1;
    }

    //after checking for filename size, we can loop through the list of dentry structs in the boot block to find the fname
    int i;
    int j;
    int file_found = 0;
    //int correct_string = 0;
    for (i = 0; i < 63; i++)    {
        for(j = 0; j < NUMBER_OF_FILE_CHARACTERS; j++){
            if (fname[j] == booting_info_block->dir_entries[i].file_name[j]){ // searching for the right file
                // printf("\n We made it into the name is correct if statement");
                //putc(fname[j]);
                if(fname[j] == NULL || j == 31){ // 31th character indicates you are at the end of the string and all characters must have matched -- fixes the 32 char edge case
                    // printf("\n made it to the end of the string\n ");
                    file_found = 1; // we know we made it to the correct string
                    break; // RECENTLY ADDED 4/2/24
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
            strcpy(dentry->file_name, booting_info_block->dir_entries[i].file_name); // copies file information into dentry
            // int k;
            // printf("debugging line: \n");
            // for(k = 0; k< 32; k++){
            //     // if(booting_info_block->dir_entries[i].file_name[k] == NULL){
            //     //     break;
            //     // }
            //     dentry->file_name[i] = booting_info_block->dir_entries[i].file_name[k]; // THIS IS THE OFFENDER IN QUESTION RN 3/25/24 5PM
            //     // putc(dentry->file_name[]);
            // }
            dentry->file_type = booting_info_block->dir_entries[i].file_type;
            dentry->inode_number = booting_info_block->dir_entries[i].inode_number;
            // Not copying over reserved, hopefully we don't care
            return 0; // return 0 because we found the file;
        }


    }
    return -1; // File must not have been found
}

/*
read_dentry_by_index
Description: This function takes in a file index in the boot block [0->62 inclusive] and populates a dentry based on the index.
Inputs: file index, and empty dentry
Outputs: None
Side Effects: the dentry is populated with the correct dentry based on the file index inputted 
*/
int32_t read_dentry_by_index(uint32_t index, dentry_struct_t * dentry){
    if(index < 0 || index > 62){
        return -1;
    } // We have a valid index

    strcpy(dentry->file_name, booting_info_block->dir_entries[index].file_name); // copies file information
    dentry->file_type = booting_info_block->dir_entries[index].file_type;
    dentry->inode_number = booting_info_block->dir_entries[index].inode_number;
    return 0;
}

/*
read_data
Description: This function takes in an inode, an offset, a buf, and a length and copies the number of bytes (length) into the buffer
Inputs: file index, and empty dentry
Outputs: None
Side Effects: the dentry is populated with the correct dentry based on the file index inputted 
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t number_of_inodes = booting_info_block->number_of_inodes;
    uint32_t number_of_data_blocks = booting_info_block->number_of_data_blocks;
    //printf("\n passes the bootblock setup variables \n");
    inode_struct_t * inode_address = (inode_struct_t*)(booting_info_block + 1 + inode);
    uint32_t actual_length_in_bytes = inode_address->length_in_bytes;
    // printf("\n The number of bytes is %d \n", actual_length_in_bytes);
   // printf("\n passes the initial setup variables \n");
    if( offset<0 || length <=0){
        printf("\n Something is off with the length and offset values \n");
        return -1;
    }
    int upperbound;
    if(offset+length > actual_length_in_bytes){ // THIS IS UNTESTED, but i think it is probably fine -- MADE CHANGES TO READ DATA
        upperbound = actual_length_in_bytes;
    }else{
        upperbound = offset+length;
    }
    // uint32_t total_number_of_data_blocks = (actual_length_in_bytes/FOUR_KB);
    int i;
    // int starting_data_block= offset/FOUR_KB; // ZERO INDEXED FROM THE LENGTH, ie if offset is zero, we at first block for the inode
    // int ending_data_block = (offset+length)/FOUR_KB; // ENDING DATA BLOCK,


    //local datablock index refers to the index of the datablock in the inode, if offset is 0, this would be 0, 
    //global datablock index gives you the index in terms of all the datablocks
    //full 4kb array index is the total array index of the structure starting with the 4kb boot block,
    //A super global index if you will
    
    // if()
    for(i = offset; i < upperbound; i++){
        uint32_t local_datablock_index = i/FOUR_KB;
        uint32_t  global_datablock_index = inode_address->data_blocks[local_datablock_index];
        // buf[i] = global_datablock_index[i%FOUR_KB];
        if(global_datablock_index >= number_of_data_blocks){
            return -1; // unlikely case, but if so might as well see whats up
        }
        uint32_t full_4kb_array_index = 1 + number_of_inodes + global_datablock_index;
        data_block_struct_t * datablock_addr = (data_block_struct_t *)booting_info_block + full_4kb_array_index;
        //putc(datablock_addr[i]);

        buf[i- offset] = datablock_addr->data[i % FOUR_KB];
        
    }
    return upperbound-offset;

}
void see_all_files_helper(){
    int i;
    for(i=0; i< 63; i++){ // 63 is max number of files
        printf("\n");
        puts(booting_info_block->dir_entries[i].file_name);
        printf(" - File Type: ");
        if(booting_info_block->dir_entries[i].file_type == RTC_FILE){
            printf("RTC File");
        }else if(booting_info_block->dir_entries[i].file_type == DIRECTORY_FILE){
            printf("Directory");
        }else if(booting_info_block->dir_entries[i].file_type == REGULAR_FILE){
            printf("Regular File");
        }
        if(i==16){ // current number of files
            printf("\n");
            printf("\n");
            break;
        }
    }
}

void terminal_init(){
    active_terminal = 0;
    terminal_processes[0] = 0;
    terminal_processes[1] = -1;
    terminal_processes[2] = -1;
    int i;
    uint8_t * t1 = (uint8_t *)0xBA000;
    uint8_t * t2 = (uint8_t *)0xBB000;
    uint8_t * t3 = (uint8_t *)0xBC000;
    for(i=0; i<4096; i++){
        *(t1+i) = (uint8_t) 0x31;
        *(t2+i) = (uint8_t) 0x32;
        *(t3+i) = (uint8_t) 0x33;
        
    }
}
