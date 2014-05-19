/* stdio.h
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

/* definitions of standard symbols in the stdio.h header file for which the
 * brainslug symbol information is available. */

#ifndef _STDIO_H_
#define _STDIO_H_

/* Functions not provided in this file:
 *  - vfprintf
 */
/* Nontstandard functions provided in this file:
 * - snprintf
 * - vsnprintf
 */
 
#include <stdarg.h>
 
#ifndef EOF
#define	EOF	(-1)
#endif
#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

/* the exact details of the FILE structure are all largely guesswork */
typedef int (*device_file_read_t)(
    int fd, void *buffer, size_t *length, void *device_ptr);
typedef int (*device_file_write_t)(
    int fd, const void *buffer, size_t *length, void *device_ptr);
typedef int (*device_file_close_t)(int fd);

typedef struct device_file {
} device_file_t;

typedef struct FILE {
    int fd; /* 0x00 */
    unsigned char unknown04[0x1c - 0x04]; /* 0x04 */
    void *buffer; /* 0x1c */
    size_t buffer_size; /* 0x20 */
    void *buffer_end; /* 0x24 */
    size_t io_size; /* 0x28 */
    unsigned char unknown2c[0x3c - 0x2c]; /* 0x2c */
    device_file_read_t read_fn; /* 0x3c */
    device_file_write_t write_fn; /* 0x40 */
    device_file_close_t close_fn; /* 0x44 */
    void *device_ptr; /* 0x48 */
    struct FILE *next_file; /* 0x4c */
} FILE;

#ifdef __GNUC__
#ifndef __ATTRIBUTE_PRINTF
#define __ATTRIBUTE_PRINTF(str, args) \
    __attribute__ ((format (printf, str, args)))
#endif
#endif

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int fprintf(FILE *stream, const char *format, ...) __ATTRIBUTE_PRINTF(2, 3);
int printf(const char *format, ...) __ATTRIBUTE_PRINTF(1, 2);
int sprintf(char *str, const char *format, ...) __ATTRIBUTE_PRINTF(2, 3);
int snprintf(char *str, size_t len, const char *format, ...)
    __ATTRIBUTE_PRINTF(3, 4);
    
int vprintf(const char *format, va_list arg);
int vsprintf(char *str, const char *format, va_list arg);
int vsnprintf(char *str, size_t len, const char *format, va_list arg);

#endif /* _STDIO_H_ */
