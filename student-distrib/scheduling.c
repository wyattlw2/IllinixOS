#include "scheduling.h"


/*  init_pit()

Description: Initializes PIT (we may or may not need this, prolly tho)
*/
void init_pit() {
    printf("Welcome. This is the PIT handler init function. Heck ye\n");
    printf("Might need to check if the PIT is actually mapped to this handler tho\n");
    return;
}


/*  pit_handler()

Description: Handles PIT interrupts. We're able to do scheduling inside of the PIT, or alternatively we can make a separate scheduling function. Would prefer the latter option
*/
void pit_handler()  {
    return;
}

