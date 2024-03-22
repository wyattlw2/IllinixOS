#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h" //Included for rtc_test
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

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//idt_test();
	//div_by_zero_test();
	//page_fault_test();
	//video_mem_test();
	//rtc_test_checkpoint_1();
	rtc_test();
}
