#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
//#include "lib.c" 
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


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
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			//while(1)
			//printf("\n OFFSET1:   %d      OFFSET 2: %d  \n", (int)idt[i].offset_15_00, (int)idt[i].offset_31_16);
			
			assertion_failure();
			result = FAIL;
			
		}
	}

	return result;
}


int exception_test(){
	TEST_HEADER;
	// while(1){
	// 	printf("\n We haven't gotten to the divide by zero part yet");
	// }
	//int temp = 5000/0;
	
	asm volatile ("int $0");
	//asm volatile ("int $1");
	// asm volatile ("int $2");
	// asm volatile ("int $3");
	// asm volatile ("int $4");
	// asm volatile ("int $5");
	// asm volatile ("int $6");
	// asm volatile ("int $7");
	// asm volatile ("int $8");
	// asm volatile ("int $9");
	// asm volatile ("int $10");
	// asm volatile ("int $11");
	// asm volatile ("int $12");
	// asm volatile ("int $13");
	// asm volatile ("int $14");
	// asm volatile ("int $15");
	// asm volatile ("int $16");
	// asm volatile ("int $17");
	// asm volatile ("int $18");
	// asm volatile ("int $19");
	//asm volatile ("int $20");
	
	//int temp = 40/0;
	// while(1){
	// 	printf("\n THIS is after the divide by zero part");
	// }
	return 0;
}

// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){

	// while(1){
	// 	printf("\n WE ARE LAUNCHING THE TEST  \n");
	// }
	//TEST_OUTPUT("idt_test", idt_test());


	TEST_OUTPUT("Exception Test: ", exception_test());

	//test_interrupts();
	// launch your tests here
}
