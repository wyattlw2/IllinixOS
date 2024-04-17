#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

void init_pit();
void pit_handler();

#endif

