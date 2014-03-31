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
#include <rvl/OSUtf.h>

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
    char __wchb[4];
  } __value;		/* Value so far.  */
} mbstate_t;

static inline size_t mbrtowc(
    wchar_t *pwc, const char *pmb, size_t max, mbstate_t *ps);
static inline size_t mbsrtowcs(
    wchar_t *dest, const char **src, size_t max, mbstate_t *ps);
static inline size_t wcrtomb(char *pmb, wchar_t wc, mbstate_t *ps);
static inline int wctob(wint_t wc);

static inline size_t mbrtowc(
        wchar_t *pwc, const char *pmb, size_t max, mbstate_t *ps) {
    static mbstate_t internal_state;
    int wchar;
    size_t bytes, i;
    char *res;
    
    if (ps == NULL)
        ps = &internal_state;
    if (pmb == NULL) {
        /* finish conversion. */
        if (ps->__count == 0)
            return (size_t)0;
        else
            return (size_t)-1;
    }
    /* We should not have a full (or overfull) buffer. */
    if (ps->__count >= sizeof(ps->__value.__wchb))
        return (size_t)-1;
    /* we need some bytes! */
    if (max == 0)
        return (size_t)-2;
    
    /* copy bytes to buffer. */
    for (i = 0;
         i < max && ps->__count < sizeof(ps->__value.__wchb);
         i++, ps->__count++) {
        ps->__value.__wchb[ps->__count] = pmb[i];
    }
    /* zero out remaining buffer space. */
    for (i = ps->__count; i < sizeof(ps->__value.__wchb); i++) {
        ps->__value.__wchb[i] = 0;
    }
    
    res = OSUTF8to32(ps->__value.__wchb, &wchar);
    
    if (res == NULL) {
        /* conversion error. */
        if (ps->__count < sizeof(ps->__value.__wchb)) {
            /* probably because we don't have enough chars. */
            return (size_t)-2;
        }
        return (size_t)-1;
    }
    
    bytes = res - ps->__value.__wchb;
    
    if (bytes > sizeof(ps->__value.__wchb)) {
        /* this should be impossible. Their implementation of OSUTF8to32. Never
         * reads beyond 4 bytes. */
        return (size_t)-1;
    }
    if (bytes > ps->__count) {
        /* this should be impossible. 0 is never valid, so our zero padding
         * should cause a conversion error. */
        return (size_t)-1;
    }
    
    /* reset shift state. */
    ps->__count = 0;
    
    if (pwc)
        *pwc = (wchar_t)wchar;
    return bytes;
}
static inline size_t mbsrtowcs(
        wchar_t *dest, const char **src, size_t max, mbstate_t *ps) {
    size_t i;
    const char *cur;
    
    cur = *src;
    for (i = 0; i < max; i++) {
        int utf32;
        const char *end;
        
        end = OSUTF8to32(cur, &utf32);
        if (end == NULL) {
            *src = cur;
            return (size_t)-1;
        }
        if (cur == end) {
            *src = NULL;
            if (dest)
                *dest = L'\0';
            return i;
        }
        
        if (dest) {
            *dest = (wchar_t)utf32;
            dest++;
        }
        cur = end;
    }
    
    *src = cur;
    return i;
}
static inline size_t wcrtomb(char *pmb, wchar_t wc, mbstate_t *ps) {
    size_t bytes;

    if ((wc  >= 0x0000D800 && wc <= 0x0000DFFF)
        || wc > 0x00200000 || wc == 0x0000FFFF || wc == 0x0000FFFE)
        return (size_t)0;

    if (wc < 0x80)
        bytes = 1;
    else if (wc < 0x800)
        bytes = 2;
    else if (wc < 0x10000)
        bytes = 3;
    else if (wc < 0x200000)
        bytes = 4;

    switch (bytes) {
    case 1:
        *pmb = (unsigned char)wc;
        break;
    case 2:
        *pmb++ = (unsigned char)((wc >> 6) | 0x000000C0);
        *pmb++ = (unsigned char)((wc & 0x0000003F) | 0x00000080);
        break;
    case 3:
        *pmb++ = (unsigned char)((wc >> 12) | 0x000000E0);
        *pmb++ = (unsigned char)(((wc >> 6) & 0x0000003F) | 0x00000080);
        *pmb++ = (unsigned char)((wc        & 0x0000003F) | 0x00000080);
        break;
    case 4:
        *pmb++ = (unsigned char)((wc >> 18)  | 0x000000F0);
        *pmb++ = (unsigned char)(((wc >> 12) & 0x0000003F) | 0x00000080);
        *pmb++ = (unsigned char)(((wc >> 6)  & 0x0000003F) | 0x00000080);
        *pmb++ = (unsigned char)((wc         & 0x0000003F) | 0x00000080);
        break;
    }

    return bytes;
}
static inline int wctob(wint_t wc) {
    return OSUTF32toANSI(wc);
}

#endif /* _WCHAR_H_ */