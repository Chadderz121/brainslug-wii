/* symbol_test.c
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
 
#ifdef _WIN32
#define FMT_SIZE "I"
#endif

#include "../src/search/symbol.c"
 
#include "symbol_test.h"

#include <stdio.h>
#include <stdint.h>

#include "../src/search/symbol.h"
#include "../src/search/fsm.h"

int SymbolTest_Parse0(void) {
    FILE *file;

    file = fopen("symbol_test_parse0.xml", "r");

    if (!file)
        return 6;
    if (!Symbol_ParseFile(file))
        return 101;
    if (symbol_count != 0)
        return 102;
        
    return 0;
}
int SymbolTest_Parse1(void) {
    FILE *file;
    symbol_t *symbol;

    file = fopen("symbol_test_parse1.xml", "r");

    if (!file)
        return 6;
    if (!Symbol_ParseFile(file))
        return 101;
    if (symbol_count != 1)
        return 102;
    
    symbol = Symbol_GetSymbol(0);
    
    if (symbol->index != 0)
        return 103;
    if (symbol->name == NULL)
        return 104;
    if (strcmp(symbol->name, "IOS_Ioctl") != 0)
        return 105;
    if (symbol->size != 0)
        return 106;
    if (symbol->offset != 0)
        return 107;
    if (symbol->data != NULL)
        return 108;
    if (symbol->mask != NULL)
        return 109;
    if (symbol->data_size != 0)
        return 110;
    if (symbol->relocation != NULL)
        return 111;
        
    return 0;
}
int SymbolTest_Parse2(void) {
    FILE *file;
    symbol_t *symbol;

    file = fopen("symbol_test_parse2.xml", "r");

    if (!file)
        return 6;
    if (!Symbol_ParseFile(file))
        return 101;
    if (symbol_count != 1)
        return 102;

    symbol = Symbol_GetSymbol(0);
    
    if (symbol->index != 0)
        return 103;
    if (symbol->name == NULL)
        return 104;
    if (strcmp(symbol->name, "IOS_Ioctl") != 0)
        return 105;
    if (symbol->size != 0x130)
        return 106;;
    if (symbol->offset != 0x4 + 0x8)
        return 107;
    if (symbol->data_size != 8)
        return 108;
    if (symbol->data == NULL)
        return 109;
    if (memcmp("ABCDEFGH", symbol->data, 8) != 0)
        return 110;
    if (symbol->mask == NULL)
        return 111;
    if (memcmp("\xff\xff\x00\x00\xff\xff\xff\xff", symbol->mask, 8) != 0)
        return 112;
    if (symbol->relocation == NULL)
        return 113;
    if (symbol->relocation->symbol == NULL)
        return 114;
    if (strcmp(symbol->relocation->symbol, "r3") != 0)
        return 115;
    if (symbol->relocation->type != R_PPC_EMB_SDA21)
        return 116;
    if (symbol->relocation->offset != 0)
        return 117;
    if (symbol->relocation->next == NULL)
        return 117;
    if (symbol->relocation->next->symbol == NULL)
        return 118;
    if (strcmp(symbol->relocation->next->symbol, "r2") != 0)
        return 119;
    if (symbol->relocation->next->type != R_PPC_ADDR16_LO)
        return 120;
    if (symbol->relocation->next->offset != 0x12c)
        return 121;
    if (symbol->relocation->next->next == NULL)
        return 122;
    if (symbol->relocation->next->next->symbol == NULL)
        return 123;
    if (strcmp(symbol->relocation->next->next->symbol, "r1") != 0)
        return 124;
    if (symbol->relocation->next->next->type != R_PPC_ADDR16_HI)
        return 125;
    if (symbol->relocation->next->next->offset != 4)
        return 126;
    if (symbol->relocation->next->next->next != NULL)
        return 127;
        
    return 0;
}
int SymbolTest_Parse3(void) {
    FILE *file;
    symbol_t *symbol;

    file = fopen("symbol_test_parse3.xml", "r");

    if (!file)
        return 6;
    if (!Symbol_ParseFile(file))
        return 101;
    if (symbol_count != 2)
        return 102;

    symbol = Symbol_GetSymbol(0);
    
    if (symbol->index != 0)
        return 103;
    if (symbol->name == NULL)
        return 104;
    if (strcmp(symbol->name, "IOS_Ioctlv") != 0)
        return 105;
    if (symbol->size != 0x130)
        return 106;
    if (symbol->offset != 0x4 + 0x8)
        return 107;
    if (symbol->data_size != 8)
        return 108;
    if (symbol->data == NULL)
        return 109;
    if (memcmp("ABCDEFGH", symbol->data, 8) != 0)
        return 110;
    if (symbol->mask == NULL)
        return 111;
    if (memcmp("\xff\xff\x00\x00\xff\xff\xff\xff", symbol->mask, 8) != 0)
        return 112;
    if (symbol->relocation == NULL)
        return 113;
    if (symbol->relocation->symbol == NULL)
        return 114;
    if (strcmp(symbol->relocation->symbol, "r3") != 0)
        return 115;
    if (symbol->relocation->type != R_PPC_EMB_SDA21)
        return 116;
    if (symbol->relocation->offset != 0)
        return 117;
    if (symbol->relocation->next == NULL)
        return 117;
    if (symbol->relocation->next->symbol == NULL)
        return 118;
    if (strcmp(symbol->relocation->next->symbol, "r2") != 0)
        return 119;
    if (symbol->relocation->next->type != R_PPC_ADDR16_LO)
        return 120;
    if (symbol->relocation->next->offset != 0x12c)
        return 121;
    if (symbol->relocation->next->next == NULL)
        return 122;
    if (symbol->relocation->next->next->symbol == NULL)
        return 123;
    if (strcmp(symbol->relocation->next->next->symbol, "r1") != 0)
        return 124;
    if (symbol->relocation->next->next->type != R_PPC_ADDR16_HI)
        return 125;
    if (symbol->relocation->next->next->offset != 4)
        return 126;
    if (symbol->relocation->next->next->next != NULL)
        return 127;
        
    symbol = Symbol_GetSymbol(1);
    
    if (symbol->index != 1)
        return 128;
    if (symbol->name == NULL)
        return 129;
    if (strcmp(symbol->name, "IOS_Ioctl") != 0)
        return 130;
    if (symbol->size != 0x130)
        return 131;
    if (symbol->offset != 0x4 + 0x8)
        return 132;
    if (symbol->data_size != 8)
        return 133;
    if (symbol->data == NULL)
        return 134;
    if (memcmp("A\0\0DEFGP", symbol->data, 8) != 0)
        return 135;
    if (symbol->mask == NULL)
        return 136;
    if (memcmp("\xff\x00\x00\x00\xff\xff\xff\xf0", symbol->mask, 8) != 0)
        return 137;
    if (symbol->relocation == NULL)
        return 138;
    if (symbol->relocation->symbol == NULL)
        return 139;
    if (strcmp(symbol->relocation->symbol, "r3") != 0)
        return 140;
    if (symbol->relocation->type != R_PPC_EMB_SDA21)
        return 141;
    if (symbol->relocation->offset != 0)
        return 142;
    if (symbol->relocation->next == NULL)
        return 143;
    if (symbol->relocation->next->symbol == NULL)
        return 144;
    if (strcmp(symbol->relocation->next->symbol, "r2") != 0)
        return 145;
    if (symbol->relocation->next->type != R_PPC_ADDR16_LO)
        return 146;
    if (symbol->relocation->next->offset != 0x12c)
        return 147;
    if (symbol->relocation->next->next == NULL)
        return 148;
    if (symbol->relocation->next->next->symbol == NULL)
        return 149;
    if (strcmp(symbol->relocation->next->next->symbol, "r1") != 0)
        return 150;
    if (symbol->relocation->next->next->type != R_PPC_ADDR16_HI)
        return 151;
    if (symbol->relocation->next->next->offset != 4)
        return 152;
    if (symbol->relocation->next->next->next != NULL)
        return 153;
        
    return 0;
}

