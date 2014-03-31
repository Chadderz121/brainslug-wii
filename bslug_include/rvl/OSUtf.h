/* OSUtf.h
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

/* definitions of symbols inferred to exist in the OSUtf.h header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_OSUTF_H_
#define _RVL_OSUTF_H_

/* Converts the utf8 encoded character at utf8 to a 32 bit utf32 integer and
 * stores it at utf32. Returns utf8 advanced by the number of characters read on
 * success, or NULL on error. If utf8 points to the NULL character, it returns
 * utf8. */
char *OSUTF8to32(const char *utf8, int *utf32);
/* Converts the utf32 character to a single byte ANSI character. Returns '\0' on
 * error. */
char OSUTF32toANSI(int utf32);

#endif /* _RVL_OSUTF_H_ */