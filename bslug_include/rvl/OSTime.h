/* OSTime.h
 *   by Alex Chadwick
 * 
 * Copyright (C) 2014, Alex Chadwick
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* definitions of symbols inferred to exist in the OSTime header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_OSTIME_H_
#define _RVL_OSTIME_H_

#include <stdint.h>

typedef struct {
    int32_t tm_sec;
    int32_t tm_min;
    int32_t tm_hour;
    int32_t tm_mday;
    int32_t tm_mon;
    int32_t tm_year;
    int32_t tm_wday;
    int32_t tm_yday;
    int32_t tm_msec;
    int32_t tm_usec;
} OSCalendarTime_t;

uint64_t OSGetTime(void);
uint32_t OSGetTick(void);
void OSTicksToCalendarTime(uint64_t ticks, OSCalendarTime_t *time);

#define TB_BUS_CLOCK				243000000u
#define TB_CORE_CLOCK				729000000u
#define TB_TIMER_CLOCK				(TB_BUS_CLOCK/4000)	

#define ticks_to_secs(ticks)		(((ticks)/(TB_TIMER_CLOCK*1000)))
#define ticks_to_millisecs(ticks)	(((ticks)/(TB_TIMER_CLOCK)))
#define ticks_to_microsecs(ticks)	((((ticks)*8)/(TB_TIMER_CLOCK/125)))
#define ticks_to_nanosecs(ticks)	((((ticks)*8000)/(TB_TIMER_CLOCK/125)))

#define secs_to_ticks(sec)			((sec)*(TB_TIMER_CLOCK*1000))
#define millisecs_to_ticks(msec)	((msec)*(TB_TIMER_CLOCK))
#define microsecs_to_ticks(usec)	(((usec)*(TB_TIMER_CLOCK/125))/8)
#define nanosecs_to_ticks(nsec)		(((nsec)*(TB_TIMER_CLOCK/125))/8000)


#endif /* _RVL_OSTIME_H_ */