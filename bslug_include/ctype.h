/* ctype.h
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

/* definitions of standard symbols in the ctype.h header file for which the
 * brainslug symbol information is available. */

#ifndef _CTYPE_H_
#define _CTYPE_H_

static inline int isalnum(int c);
static inline int isalpha(int c);
static inline int iscntrl(int c);
static inline int isdigit(int c);
static inline int isgraph(int c);
static inline int islower(int c);
static inline int isprint(int c);
static inline int ispunct(int c);
static inline int isspace(int c);
static inline int isupper(int c);
static inline int isxdigit(int c);

static inline int tolower(int c);
static inline int toupper(int c);

enum ctype_t {
    CTYPE_ISALPHA = 0x0001,
    CTYPE_ISBLANK = 0x0002,
    CTYPE_ISCNTRL = 0x0004,
    CTYPE_ISDIGIT = 0x0008,
    CTYPE_ISGRAPH = 0x0010,
    CTYPE_ISLOWER = 0x0020,
    CTYPE_ISPRINT = 0x0040,
    CTYPE_ISPUNCT = 0x0080,
    CTYPE_ISSPACE = 0x0100,
    CTYPE_ISUPPER = 0x0200,
    CTYPE_ISXDIGIT = 0x0400,
    CTYPE_ISALNUM = CTYPE_ISALPHA | CTYPE_ISDIGIT
};

extern const short __ctype[256];
extern const char  __lcase[256];
extern const char  __ucase[256];

static inline int isalnum(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISALNUM);
}
static inline int isalpha(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISALPHA);
}
static inline int isblank(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISBLANK);
}
static inline int iscntrl(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISCNTRL);
}
static inline int isdigit(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISDIGIT);
}
static inline int isgraph(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISGRAPH);
}
static inline int islower(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISLOWER);
}
static inline int isprint(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISPRINT);
}
static inline int ispunct(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISPUNCT);
}
static inline int isspace(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISSPACE);
}
static inline int isupper(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISUPPER);
}
static inline int isxdigit(int c) {
    return c >= 0 && c < sizeof(__ctype) && (__ctype[c] & CTYPE_ISXDIGIT);
}

static inline int tolower(int c) {
    if (c < 0 || c >= sizeof(__lcase))
        return c;
    return __lcase[c];
}
static inline int toupper(int c) {
    if (c < 0 || c >= sizeof(__ucase))
        return c;
    return __ucase[c];
}


#endif /* _CTYPE_H_ */