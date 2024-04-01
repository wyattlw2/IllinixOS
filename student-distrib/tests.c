#include "tests.h"
#include "x86_desc.h"
#include "terminal.h"
#include "lib.h"
#include "rtc.h" //Included for rtc_test
#include "file_sys_driver.h"
//#include "lib.c" 
#define PASS 1
#define FAIL 0


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

// static inline void assertion_failure(){
// 	/* Use exception #15 for assertions, otherwise
// 	   reserved by Intel */
// 	asm volatile("int $15");
// }


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
void idt_test(){
	int i;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			//while(1)
			//printf("\n OFFSET1:   %d      OFFSET 2: %d  \n", (int)idt[i].offset_15_00, (int)idt[i].offset_31_16);
			printf("\n IDT Test Failed \n");
			return;
		}
	}
	printf("\n IDT Test Failed \n");
	return;
}
//EXCEPTION ZERO TEST: Divide By Zero -- Will cause an exception 0
void div_by_zero_test(){
	int i = 1;
	int j = 0;
	int k = i/j;
	k++;
}
// Page Fault Test: will cause an exception 14
void page_fault_test(){
	int * pointer = (int *) 0x00000001;
	int temp = *pointer;
	temp++;
}
void video_mem_test(){
	int * pointer = (int *) 0xB8001;
	int temp = *pointer;
	temp++;
	printf("\n The Video Memory Check Passed \n");
}

void terminal_test() {
	char b[128];
	while(1) {
		t_read(0,b,128);
		t_write(0,b,128);
	}
}

void rtc_test_checkpoint_1(){
	// HAVE TO PUT test_interrupts() function inside the rtc driver
}


// add more tests here

/* Checkpoint 2 tests */
void rtc_test(){
	//init_rtc(); // init_rtc() is already called prior to launch_tests()
	rtc_open(0); //Frequency is set to 2 Hz
	int j;
	// for(j = 0; j < 4; j++) {
	// 	rtc_read(0,(void*)0,0);
	// }

	//printf("Before For loop");
	clear();

	int i;
	for(i = 1; i <= 1024; i *= 2){
		//int freq = 2^i;
		
		printf("Frequency: %d", i);
		printf("\n");
		// for(j = 0; j < 4; j++)
		// 	rtc_read(0,(void*)0,0);
		//printf("Waiting");
		rtc_write(0,(void*)i,4);
		//printf("Written");
		for(j = 0; j < 4*i; j++)
			rtc_read(0,(void*)0,0);
		//printf("More Waiting");
		clear();
		// for(j = 0; j < 10; j++)
		// 	rtc_read(0,(void*)0,0);
	}

	rtc_open(0);
	rtc_close(0);
	clear();
	printf("\n The RTC Check Passed \n");
	//disable_irq(8);
	return;
}
void read_dentry_by_name_test(){
	clear();
    uint8_t test_name[32] = {"frame0.txt"};
	printf("\n The name of the file is: ");
	puts((int8_t*)test_name);
    dentry_struct_t sample_dentry; 
    int retval = read_dentry_by_name(test_name, &sample_dentry);
	printf("\n Inside the directory entry, the name of the file is: ");
	puts((int8_t*)sample_dentry.file_name);
    printf("\n The inode number of this file is %d \n", sample_dentry.inode_number);
	printf("\n The error code for read_dentry_by_name_test is: %d \n", retval);
}

void read_dentry_by_index_test(){
	clear();
    //uint8_t test_name[32] = {"frame0.txt"};
	uint32_t index = 8;
	printf("\n \n \n \n \n");
	printf("\n The index of the file being checked is: %d \n", index);
	//puts(test_name);
    dentry_struct_t sample_dentry; 
    int retval = read_dentry_by_index(index, &sample_dentry);
	printf("\n Inside the Directory Entry, the name of the file is: ");
	puts(sample_dentry.file_name);
    printf("\n The Inode Number of this file is %d \n", sample_dentry.inode_number);
	printf("\n The return value of this function is: %d \n", retval);
}

void read_data_test(){
	clear();
    //uint8_t test_name[32] = {"frame0.txt"};
	uint32_t inode_num = 38; // Frame1.txt
	printf("\n \n \n \n \n");
	printf("\n The inode of the file being checked is: %d \n", inode_num);
	//puts(test_name);
    //dentry_struct_t sample_dentry; 
	uint8_t buffer[200];
    int retval = read_data(inode_num, 0, buffer, 187);
	printf("This is the return value of the function: %d \n", retval);
	printf("\n This is what's inside the file: \n" );
	puts((int8_t*)buffer);
}







//TESTS FOR FILE OPEN/READ/WRITE/CLOSE BELOW
void file_open_test(){
	clear();
	printf("FILE_OPEN TEST.\n \n \n");
    	//set this line to whichever filename you want.
											//i have NOT tested directory filenames yet. should theoretically be fine however
	uint8_t test_name[32] = {"frame0.txt"};
	printf(" Opening file ");
	puts((int8_t*)test_name);
	// printf("...\n");
	printf(" \n ");
    dentry_struct_t sample_dentry; 
    int retval = file_open(test_name);
	printf("\n Successfully called file_open without crashing.");
	if (retval == -1)
	{
		printf("\n\n However, the system either \n a.) could not locate the inputted filename,\n");
		printf(" b.) the opened file is of type 'directory' when it should be 'regular file', or");
		printf(" c.) the inputted filename exceeds 32 characters.\n\n");
		return;
	}

	printf("\n The name of the opened file is: ");
	puts((int8_t*)(&sample_dentry)->file_name);
    printf(".\n The inode number of this file is %d. \n", (&sample_dentry)->inode_number);
	printf("\n The error code for file_open is: %d. \n", retval);
}

void file_read_test(){
	clear();
	uint8_t buffer[36000];
	// uint8_t filename[32] = {"created.txt"};
	uint8_t filename[32] = {"frame0.txt"};
	// uint8_t filename[32] = {"shell"};
	// uint8_t filename[32] = {"verylargetextwithverylongname.tx"};
	printf("\n \n \n \n \n Starting File Read Test, Attempting to Read File: \n ");
	puts((int8_t*)filename);
	dentry_struct_t dentry_to_read;
	int retval = file_open(filename);
	if(retval == 0){
		printf("\n File Open Successful, attempting read \n");
	}else{
		printf("\n File Open Failed, please try again \n");
		return;
	}
	// printf("\n Number of Bytes in the whole file: %d \n", dentry_to_read.)
	// int num_bytes = 21; // for created.txt
	// int num_bytes = 5277;// for verylargetext...etc 
	int num_bytes = 187;
	int retval2 = file_read(&dentry_to_read, buffer, num_bytes);
	if(retval2 == 0){
		printf("\n File Read was successful, outputting the contents of the file: \n");
		puts((int8_t*)buffer);
		// int i;
		// for(i=0; i< num_bytes; i++){ // USE THIS IF BINARY FILES TEST
		// 	if(buffer[i] == NULL){
		// 		continue;
		// 	}
		// 	putc(buffer[i]);
		// }
	} else{
		printf("\n File Read Failed \n");
	}
}
void see_all_files(){
	//printf("\n");	//probably written before terminal scrolling feature was merged with this code
	clear();
	printf("\n DUMPING ALL FILES FROM BOOT BLOCK...\n");
	see_all_files_helper();
}
void file_write_test(){
	clear();
	int retval = file_write();
	if(retval == -1){
		printf("\n File Write Failed, Meaning we Passed the test :)\n\n");
	}
}

void file_close_test(){
	clear();
	//printf("\n \n \n \n \n");
	printf("\n Attempting File Close Test: \n");
	printf(" First attempting to open the file...");
	dentry_struct_t dentry_to_close;
	uint8_t filename[32] = {"frame0.txt"};
	int retval = file_open(filename);
	if(retval == 0){
		printf("\n File Open Successful! \n");
	}else{
		printf("\n File Open Failed, please try again. \n");
		return;
	}
	printf("\n Attempting to close the file \n");
	int retval2 = file_close(&dentry_to_close);
	if(retval2 == 0){
		printf("\n Successfully called file_close(), test passed \n");
	}else{
		printf("Something happened and the test failed, please try again \n");
	}
}
void directory_open_close_test(){
	clear();
	//printf("\n \n \n \n \n");
	printf("\n Attempting Directory Open Test: \n");
	
	dentry_struct_t dentry_to_open;
	uint8_t filename[32] = {"."};
	printf("\n Attempting to open directory: ");
	puts((int8_t*)filename);
	int retval = directory_open(filename, &dentry_to_open);
	if(retval == 0){
		printf("\n Directory Open Successful! \n");
	}else{
		printf("\n Directory Open Failed, please try again \n");
		return;
	}


	printf("\n Attempting to close the file \n");
	int retval2 = directory_close(&dentry_to_open);
	if(retval2 == 0){
		printf("\n File Closure Successful, test passed \n");
	}else{
		printf("Something happened and the test failed, please try again");
	}
}
void directory_write_test(){
	clear();
	int retval = directory_write();
	if(retval == -1){
		printf("\n Directory Write Test Failed, Meaning we Passed the test :)\n\n");
	}
}
void directory_read_test(){
	clear();
	//printf("\n \n \n \n \n");
	uint8_t buffer[3];		//anywhere where i changed the variable type was to remove compiler warnings
	dentry_struct_t dentry_to_open;
	uint8_t filename[32] = {"."};
	printf("\n Attempting to open directory: ");
	puts((int8_t*)filename);
	int retval = directory_open(filename, &dentry_to_open);
	if(retval == 0){
		printf("\n Directory Open Successful! \n");
	}else{
		printf("\n Directory Open Failed, please try again \n");
		return;
	}
	// clear();
	printf("\n Attempting a Directory Read \n");
	int nbytes = 1;
	int retval2 = directory_read(&dentry_to_open, buffer, nbytes);
	if(retval2 == 0){
		printf("\n Directory Read Successful, the name of the directory is: ");
		puts((int8_t*)buffer);
	}else{
		printf("\n Directory Read Failed \n");
	}
}




/* Checkpoint 3 tests */

void system_call_test_basic(){
	asm volatile("movl $5, %eax");	//this actually works. extremely pog
	asm volatile("movl $1, %ebx");	//this actually works. extremely pog
	asm volatile("movl $2, %ecx");	//this actually works. extremely pog
	asm volatile("movl $3, %edx");	//this actually works. extremely pog
	asm volatile("int $0x80");
}

void execute_test_file_load() {
    int8_t var[32] = {"shell"};
    
    asm volatile (
        "movl %0, %%ebx;"   // Move the address of var into register ebx
        :                   // Output operand list is empty
        : "r" (var)         // Input operand list, specifying that var is an input
    );

    asm volatile (
        "movl $2, %eax"     // Set syscall number to 2 (sys_exec)
    );

    // For demonstration purposes only, as usage of int $0x80 is system-dependent
    asm volatile (
        "int $0x80"         // Execute syscall
    );
}
void function_pointers_test(){
	int8_t var[32] = {"frame0.txt"};
    
    asm volatile (
        "movl %0, %%ebx;"   // Move the address of var into register ebx
        :                   // Output operand list is empty
        : "r" (var)         // Input operand list, specifying that var is an input
    );

    asm volatile (
        "movl $5, %eax"     // Set syscall number to 2 (sys_open)
    );

    // For demonstration purposes only, as usage of int $0x80 is system-dependent
    asm volatile (
        "int $0x80"         // Execute syscall
    );
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){

	//CHECKPOINT 1:
	//idt_test();
	//div_by_zero_test();
	//page_fault_test();
	//video_mem_test();
	//rtc_test_checkpoint_1();

	//CHECKPOINT 2:
	// rtc_test(); // REMEMBER TO UNCOMMENT THE PUTC IN THE RTC HANDLER
	//read_dentry_by_name_test();
	//read_dentry_by_index_test();
	//read_data_test();
	//  file_open_test();
	// file_read_test();
	// see_all_files();
	// file_write_test();
	// file_close_test();
	// directory_open_close_test();
	// directory_write_test();
	// directory_read_test();
	// terminal_test();

	//CHECKPOINT 3:
	// system_call_test_basic();
	execute_test_file_load();
	// function_pointers_test();

}

