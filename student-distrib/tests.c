#include "tests.h"
#include "x86_desc.h"
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
	puts((uint8_t*)test_name);
    dentry_struct_t sample_dentry; 
    int retval = read_dentry_by_name(test_name, &sample_dentry);
	printf("\n Inside the Directory Entry, The name of the file is: ");
	puts((int8_t*)sample_dentry.file_name);
    printf("\n The Inode Number of this file is %d \n", sample_dentry.inode_number);
	printf("\n The return value of this function is: %d \n", retval);
}

void read_dentry_by_index_test(){
	clear();
    //uint8_t test_name[32] = {"frame0.txt"};
	uint32_t index = 13;
	printf("\n \n \n \n \n");
	printf("\n The index of the file being checked is: %d", index);
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
	uint32_t inode_num = 38;
	printf("\n \n \n \n \n");
	printf("\n The inode of the file being checked is: %d", inode_num);
	//puts(test_name);
    //dentry_struct_t sample_dentry; 
	uint8_t buffer[100];
    int retval = read_data(inode_num, 0, buffer, 100);
	printf("This is the return value of the function: %d", retval);
	printf("\n This is what's inside the file: \n" );
	puts((int8_t*)buffer);
}



/* Checkpoint 3 tests */
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
	//rtc_test();
	// read_dentry_by_name_test();
	//read_dentry_by_index_test();
	read_data_test();
}

