/* errno.h
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

/* definitions of standard symbols in the fcntl.h. These methods are
 * provided in the libc module. */

#ifndef _FCNTL_H_
#define _FCNTL_H_

#define _FREAD      0x0001  /* read enabled */
#define _FWRITE     0x0002  /* write enabled */
#define _FAPPEND    0x0008  /* append (writes guaranteed at the end) */
#define _FCREAT     0x0200  /* open with file create */
#define _FTRUNC     0x0400  /* open with truncation */
#define _FEXCL      0x0800  /* error on open if file exists */
#define _FSYNC      0x2000  /* do all writes synchronously */

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_APPEND    _FAPPEND
#define O_CREAT     _FCREAT
#define O_TRUNC     _FTRUNC
#define O_EXCL      _FEXCL
#define O_SYNC      _FSYNC

#define O_NONBLOCK  0x0800 /* non blocking (sockets only) */

#define F_GETFL     3   /* Get file flags */
#define F_SETFL     4   /* Set file flags */

#endif /* _FCNTL_H_ */