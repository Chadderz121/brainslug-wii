/* sys/types.h
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

/* definitions of standard symbols in the sys/types.h header file. */

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stdint.h>

typedef	unsigned int    ino_t;
typedef long            off_t;
typedef short           dev_t;
typedef unsigned short  uid_t;
typedef unsigned short  gid_t;
typedef int             ssize_t;
typedef unsigned int    mode_t;
typedef unsigned short  nlink_t;
typedef uint64_t        time_t;
typedef uint32_t        fsblkcnt_t;
typedef uint32_t        fsfilcnt_t;

#endif /* _SYS_TYPES_H_ */