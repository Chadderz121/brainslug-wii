/* symbol.h
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
 
#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef unsigned int symbol_index_t;
typedef unsigned int symbol_alphabetical_index_t;

typedef struct symbol_relocation_t {
    const char *symbol;
    unsigned char type;
    size_t offset;
    const struct symbol_relocation_t *next;
} symbol_relocation_t;

typedef struct {
    symbol_index_t index;
    const char *name;
    size_t size;
    size_t offset;
    const uint8_t *data;
    const uint8_t *mask;
    size_t data_size;
    bool debugging;
    const symbol_relocation_t *relocation;
} symbol_t;

#define SYMBOL_NULL ((symbol_index_t)0xffffffff)

extern symbol_index_t symbol_count;

symbol_t *Symbol_GetSymbol(symbol_index_t index);
symbol_t *Symbol_GetSymbolSize(symbol_index_t index);
symbol_t *Symbol_GetSymbolAlphabetical(symbol_alphabetical_index_t index);
symbol_alphabetical_index_t Symbol_SearchSymbol(const char *name);
bool Symbol_ParseFile(FILE *file);

#endif /* SYMBOL_H_ */
