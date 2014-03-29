/* search.h
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

/* This file should ideally avoid Wii specific methods so unit testing can be
 * conducted elsewhere. */
 
#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdbool.h>

#include "library/event.h"

extern event_t search_event_complete;
extern bool search_has_error;
/* whether or not to delay loading for debug messages. */
extern bool search_has_info;

bool Search_Init(void);
bool Search_RunBackground(void);

bool Search_SymbolAdd(const char *name, void *address);
bool Search_SymbolReplace(const char *name, void *address);
void *Search_SymbolLookup(const char *name);

#endif /* SEARCH_H_ */
