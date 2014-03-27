/* wchar.h
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

/* definitions of standard symbols in the wchar.h. These methods are provided in
 * the libc module. */

#ifndef _WCHAR_H_
#define _WCHAR_H_

#define __need_wint_t
#include <stddef.h>
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN 0
#endif

#ifndef WCHAR_MAX
#define WCHAR_MAX 0xffffu
#endif

/* Conversion state information.  */
typedef struct {
  int __count;
  union {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;		/* Value so far.  */
} mbstate_t;

size_t mbrtowc(wchar_t* pwc, const char* pmb, size_t max, mbstate_t* ps);
size_t mbsrtowcs(wchar_t* dest, const char** src, size_t max, mbstate_t* ps);
size_t wcrtomb(char* pmb, wchar_t wc, mbstate_t* ps);
int wctob(wint_t wc);

#endif /* _WCHAR_H_ */