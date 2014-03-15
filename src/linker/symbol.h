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

#include <stddef.h>
#include <stdint.h>

typedef enum {
    R_PPC_NONE = 0,
    R_PPC_ADDR32 = 1,
    R_PPC_ADDR24 = 2,
    R_PPC_ADDR16 = 3,
    R_PPC_ADDR16_LO = 4,
    R_PPC_ADDR16_HI = 5,
    R_PPC_ADDR16_HA = 6,
    R_PPC_ADDR14 = 7,
    R_PPC_ADDR14_BRTAKEN = 8,
    R_PPC_ADDR14_BRNTAKEN = 9,
    R_PPC_REL24 = 10,
    R_PPC_REL14 = 11,
    R_PPC_REL14_BRTAKEN = 12,
    R_PPC_REL14_BRNTAKEN = 13,
    R_PPC_GOT16 = 14,
    R_PPC_GOT16_LO = 15,
    R_PPC_GOT16_HI = 16,
    R_PPC_GOT16_HA = 17,
    R_PPC_PLTREL24 = 18,
    R_PPC_COPY = 19,
    R_PPC_GLOB_DAT = 20,
    R_PPC_JMP_SLOT = 21,
    R_PPC_RELATIVE = 22,
    R_PPC_LOCAL24PC = 23,
    R_PPC_UADDR32 = 24,
    R_PPC_UADDR16 = 25,
    R_PPC_REL32 = 26,
    R_PPC_PLT32 = 27,
    R_PPC_PLTREL32 = 28,
    R_PPC_PLT16_LO = 29,
    R_PPC_PLT16_HI = 30,
    R_PPC_PLT16_HA = 31,
    R_PPC_SDAREL16 = 32,
    R_PPC_SECTOFF = 33,
    R_PPC_SECTOFF_LO = 34,
    R_PPC_SECTOFF_HI = 35,
    R_PPC_SECTOFF_HA = 36,
    R_PPC_ADDR30 = 37,
    R_PPC_EMB_NADDR32 = 101,
    R_PPC_EMB_NADDR16 = 102,
    R_PPC_EMB_NADDR16_LO = 103,
    R_PPC_EMB_NADDR16_HI = 104,
    R_PPC_EMB_NADDR16_HA = 105,
    R_PPC_EMB_SDAI16 = 106,
    R_PPC_EMB_SDA2I16 = 107,
    R_PPC_EMB_SDA2REL = 108,
    R_PPC_EMB_SDA21 = 109,
    R_PPC_EMB_MRKREF = 110,
    R_PPC_EMB_RELSEC16 = 111,
    R_PPC_EMB_RELST_LO = 112,
    R_PPC_EMB_RELST_HI = 113,
    R_PPC_EMB_RELST_HA = 114,
    R_PPC_EMB_BIT_FLD = 115,
    R_PPC_EMB_RELSDA = 116,
} relocation_t;

typedef unsigned int symbol_index_t;

typedef struct symbol_relocation_t {
    const char *symbol;
    relocation_t type;
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
    const symbol_relocation_t *relocation;
} symbol_t;

extern symbol_index_t symbol_count;

symbol_t *Symbol_GetSymbol(symbol_index_t index);

#endif /* SYMBOL_H_ */
