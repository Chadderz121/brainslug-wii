/* string.h
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

/* definitions of standard symbols in the string.h header file for which the
 * brainslug symbol information is available. */

#ifndef _STRING_H_
#define _STRING_H_

/* Functions not provided in this file:
 *  - strncat
 *  - strcoll
 *  - strcspn
 *  - strerror
 */
/* Nontstandard functions provided in this file:
 * - memrchr
 */

#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

void *memcpy(void *dst, const void *src, size_t num);
void *memchr(const void *ptr, int value, size_t num); 
void *memrchr(const void *ptr, int value, size_t num); 
int memcmp(const void *ptr1, const void *ptr2, size_t num);
void *memmove(void *dst, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t maxlen);
char *strcat(char *dst, const char *src);
char *strchr(const char *str, int character);
int strcmp (const char *str1, const char *str2);
size_t strlen(const char *s);
int strncmp (const char *str1, const char *str2, size_t maxlen);
size_t strnlen(const char *s, size_t maxlen);

static inline char *strrchr(const char *str, int character) {
    return memrchr(str, character, strlen(str) + 1);
}
static inline char *strpbrk(const char *str, const char *set) {
    char *result = (char *)str;
    for (; result[0] != '\0'; result++)
        if (strchr(set, result[0]) != NULL)
            return result;
    return NULL;
}

#endif /* _STRING_H_ */