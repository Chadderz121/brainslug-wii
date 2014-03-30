/* time.h
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

/* definitions of standard symbols in the time.h. These methods are provide in
 * the libc module. */

#ifndef _TIME_H_
#define _TIME_H_

/* Functions not provided in this file:
 *  - asctime
 *  - ctime
 *  - strftime
 */
/* Nontstandard functions provided in this file:
 * - localtime_r
 * - gmtime_r
 *
 * The implementation of gmtime is to return localtime in all cases.
 */

#include <rvl/OSTime.h>
#include <stddef.h>
#include <sys/types.h>

#define CLOCKS_PER_SEC 1000

typedef unsigned long clock_t;

struct tm {
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
};

static inline clock_t clock(void) {
    return ticks_to_millisecs(OSGetTime());
}
static inline double difftime(time_t end, time_t beginning) {
    return ticks_to_secs((double)(end - beginning));
}
/* naughtily, I ignore daylight savings! */
static inline time_t mktime(struct tm *tim_p) {
    /* this needs verifying! */
#define _SEC_IN_MINUTE 60L
#define _SEC_IN_HOUR 3600L
#define _SEC_IN_DAY 86400L

static const int _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

    time_t tim = 0;
    long days = 0;
    int year;
    
    /* compute hours, minutes, seconds */
    tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) +
        (tim_p->tm_hour * _SEC_IN_HOUR);
    /* compute days in year */
    days += tim_p->tm_mday - 1;
    days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
    if (tim_p->tm_mon > 1 && _DAYS_IN_YEAR (tim_p->tm_year) == 366)
        days++;
        
    if ((year = tim_p->tm_year) > 100) {
        for (year = 100; year < tim_p->tm_year; year++)
            days += _DAYS_IN_YEAR (year);
    } else if (year < 100) {
        for (year = 99; year > tim_p->tm_year; year--)
            days -= _DAYS_IN_YEAR (year);
        days -= _DAYS_IN_YEAR (year);
    }
    tim += (days * _SEC_IN_DAY);
    
    return secs_to_ticks(tim);
}
static inline time_t time(time_t *timer) {
    time_t clk = OSGetTime();
    if (timer != NULL) *timer = clk;
    return clk;
}
static inline struct tm *localtime(const time_t *timer) {
    static struct tm date;
    OSCalendarTime_t ct;
    
    OSTicksToCalendarTime(*timer, &ct);
    date.tm_sec = ct.tm_sec;
    date.tm_min = ct.tm_min;
    date.tm_hour = ct.tm_hour;
    date.tm_mday = ct.tm_mday;
    date.tm_mon = ct.tm_mon;
    date.tm_year = ct.tm_year;
    date.tm_wday = ct.tm_wday;
    date.tm_yday = ct.tm_yday;
    date.tm_isdst = 0;
    return &date;
}
#define gmtime localtime

static inline struct tm *localtime_r(const time_t *timer, struct tm *date) {
    OSCalendarTime_t ct;
    
    OSTicksToCalendarTime(*timer, &ct);
    date->tm_sec = ct.tm_sec;
    date->tm_min = ct.tm_min;
    date->tm_hour = ct.tm_hour;
    date->tm_mday = ct.tm_mday;
    date->tm_mon = ct.tm_mon;
    date->tm_year = ct.tm_year;
    date->tm_wday = ct.tm_wday;
    date->tm_yday = ct.tm_yday;
    date->tm_isdst = 0;
    return date;
}
#define gmtime_r localtime_r

#endif /* _TIME_H_ */