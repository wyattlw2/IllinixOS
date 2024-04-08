#ifndef _RTC_H_
#define _RTC_H_
#include "types.h"
#include "lib.h"
#include "i8259.h"

int32_t rtc_int;

void init_rtc(void);
void rtc_handler();
int32_t rtc_set_frequency(int32_t frequency);
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close (int32_t fd);
int32_t vrtc_process (int32_t fd, void * buf, int32_t nbytes);
#endif
