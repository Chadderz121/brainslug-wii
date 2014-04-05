/* symbol.c
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
 
#include "symbol.h"

#include <assert.h>
#include <elfdefinitions.h>
#include <mxml.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t data_size;
    symbol_index_t index;
} symbol_size_index_entry_t;

typedef struct {
    const char *name;
    symbol_index_t index;
} symbol_alphabetical_index_entry_t;

const struct {
    unsigned char relocation;
    const char *name;
    uint8_t mask[4];
} symbol_relocation_strings[] = {
    { R_PPC_ADDR32, "addr", { 0x00, 0x00, 0x00, 0x00 } },
    { R_PPC_ADDR16_LO, "lo", { 0xff, 0xff, 0x00, 0x00 } },
    { R_PPC_ADDR16_HI, "hi", { 0xff, 0xff, 0x00, 0x00 } },
    { R_PPC_ADDR16_HA, "ha", { 0xff, 0xff, 0x00, 0x00 } },
    { R_PPC_REL24, "b", { 0xfc, 0x00, 0x00, 0x03 } },
    { R_PPC_REL14, "bc", { 0xff, 0xff, 0x00, 0x03 } },
    { R_PPC_EMB_SDA21, "sda21", { 0xff, 0xe0, 0x00, 0x00 } },
};

#ifndef FMT_SIZE
#define FMT_SIZE ""
#endif

#define SYMBOL_RELOCATION_STRINGS_COUNT \
    (sizeof(symbol_relocation_strings) / sizeof(*symbol_relocation_strings))
    
#define SYMBOL_LIST_INITIAL_CAPACITY 128

symbol_index_t symbol_count = 0;

static symbol_t *symbol_globals = NULL;
static symbol_t *symbol_globals_end = NULL;
static symbol_t *symbol_globals_free = NULL;

static symbol_size_index_entry_t *symbol_size_index = NULL;
static symbol_alphabetical_index_entry_t *symbol_alphabetical_index = NULL;

static symbol_t *Symbol_AllocSymbol(const char *name, size_t name_length);
static symbol_relocation_t *Symbol_AddRelocation(
    symbol_t *symbol, const char *target,
    unsigned char type, size_t offset);
static int Symbol_CompareSize(const void *left_ptr, const void *right_ptr);
static int Symbol_CompareName(const void *left_ptr, const void *right_ptr);

symbol_t *Symbol_GetSymbol(symbol_index_t index) {
    assert(symbol_globals != NULL);

    return &symbol_globals[index];
}

bool Symbol_ParseFile(FILE *file) {
    bool result = false, set_debug;
    mxml_node_t *xml_tree = NULL;
    mxml_node_t *xml_symbols = NULL;
    mxml_node_t *xml_symbol = NULL;
    const char *debug;

    xml_tree = mxmlLoadFile(NULL, file, MXML_TEXT_CALLBACK);
    if (xml_tree == NULL)
        goto exit_error;
    /* <symbols> root element */
    xml_symbols = mxmlFindElement(
        xml_tree, xml_tree, "symbols", NULL, NULL, MXML_DESCEND_FIRST);
    if (xml_symbols == NULL)
        goto exit_error;
    debug = mxmlElementGetAttr(xml_symbols, "debug");
    set_debug = debug != NULL && strcmp(debug, "on") == 0;
    
    /* <symbol name=""> element within symbols */
    xml_symbol = mxmlFindElement(
        xml_symbols, xml_symbols, "symbol", NULL, NULL, MXML_DESCEND_FIRST);
    while (xml_symbol != NULL) {
        const char *name, *size_str, *offset_str;
        mxml_node_t *xml_data = NULL, *xml_reloc = NULL;
        symbol_t *symbol;
        uint8_t *data, *mask;

        name = mxmlElementGetAttr(xml_symbol, "name");
        size_str = mxmlElementGetAttr(xml_symbol, "size");
        offset_str = mxmlElementGetAttr(xml_symbol, "offset");

        if (name == NULL)
            goto next_symbol;

        symbol = Symbol_AllocSymbol(name, strlen(name));

        if (symbol == NULL)
            goto exit_error;

        symbol->debugging = set_debug;
            
        if (size_str != NULL) {
            if (sscanf(size_str, "%" FMT_SIZE "x", &symbol->size) != 1 && 
                sscanf(size_str, "%" FMT_SIZE "u", &symbol->size) != 1)

                goto next_symbol;
        } else
            symbol->size = 0;
        if (offset_str != NULL) {
            if (sscanf(offset_str, "%" FMT_SIZE "x", &symbol->offset) != 1 &&
                sscanf(offset_str, "%" FMT_SIZE "d", &symbol->offset) != 1)
                
                goto next_symbol;
        } else
            symbol->offset = 0;

        /* <data>FF</data> */
        xml_data = mxmlFindElement(
            xml_symbol, xml_symbol, "data", NULL, NULL, MXML_DESCEND_FIRST);
        if (xml_data != NULL) {
            mxml_node_t *xml_value;
            size_t data_size;
            unsigned int i;

            data_size = 0;
            xml_value = xml_data->child;
            while (xml_value != NULL &&
                   xml_value->type == MXML_TEXT &&
                   xml_value->value.text.string != NULL) {
                for (i = 0; xml_value->value.text.string[i] != '\0'; i++) {
                    switch (xml_value->value.text.string[i]) {
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                        case 'a': case 'b': case 'c':
                        case 'd': case 'e': case 'f':
                        case 'A': case 'B': case 'C':
                        case 'D': case 'E': case 'F':
                        case '?':
                            data_size++;
                            break;
                        case ' ': case '\t': case '\r': case '\n':
                            break;
                        default:
                            goto next_symbol;
                    }
                }
                xml_value = xml_value->next;
            }

            /* symbol must be in bytes, so two hex digits per byte! */
            if (data_size % 2 != 0)
                goto next_symbol;

            symbol->data = data = malloc(data_size);
            symbol->data_size = data_size / 2;
            symbol->mask = mask = data + (data_size / 2);

            if (data == NULL)
                goto next_symbol;

            data_size = 0;
            xml_value = xml_data->child;
            while (xml_value != NULL &&
                   xml_value->type == MXML_TEXT &&
                   xml_value->value.text.string != NULL) {
                for (i = 0; xml_value->value.text.string[i] != '\0'; i++) {
                    switch (xml_value->value.text.string[i]) {
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                            data[data_size / 2] =
                                (data[data_size / 2] << 4) +
                                (xml_value->value.text.string[i] - '0');
                            mask[data_size / 2] =
                                (mask[data_size / 2] << 4) + 0xf;
                            data_size++;
                            break;
                        case 'a': case 'b': case 'c':
                        case 'd': case 'e': case 'f':
                            data[data_size / 2] =
                                (data[data_size / 2] << 4) +
                                (xml_value->value.text.string[i] - 'a' + 10);
                            mask[data_size / 2] =
                                (mask[data_size / 2] << 4) + 0xf;
                            data_size++;
                            break;
                        case 'A': case 'B': case 'C':
                        case 'D': case 'E': case 'F':
                            data[data_size / 2] =
                                (data[data_size / 2] << 4) +
                                (xml_value->value.text.string[i] - 'A' + 10);
                            mask[data_size / 2] =
                                (mask[data_size / 2] << 4) + 0xf;
                            data_size++;
                            break;
                        case '?':
                            data[data_size / 2] =
                                (data[data_size / 2] << 4) + 0x0;
                            mask[data_size / 2] =
                                (mask[data_size / 2] << 4) + 0x0;
                            data_size++;
                            break;
                        case ' ': case '\t': case '\r': case '\n':
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
                xml_value = xml_value->next;
            }
        } else {
            symbol->data = data = NULL;
            symbol->mask = mask = NULL;
            symbol->data_size = 0;
        }

        /* <reloc type="" offset="" symbol="" /> */
        xml_reloc = mxmlFindElement(
            xml_symbol, xml_symbol, "reloc", NULL, NULL, MXML_DESCEND_FIRST);
        while (xml_reloc != NULL) {
            const char *type_str, *offset_str, *symbol_str;
            symbol_relocation_t *relocation;
            unsigned char type = R_PPC_NONE;
            size_t offset;
            const uint8_t *relocation_mask = NULL;
            int i;

            type_str = mxmlElementGetAttr(xml_reloc, "type");
            offset_str = mxmlElementGetAttr(xml_reloc, "offset");
            symbol_str = mxmlElementGetAttr(xml_reloc, "symbol");

            if (type_str == NULL || offset_str == NULL)
                goto next_reloc;

            for (i = 0; i < SYMBOL_RELOCATION_STRINGS_COUNT; i++) {
                if (strcasecmp(
                    type_str, symbol_relocation_strings[i].name) == 0) {
                    type = symbol_relocation_strings[i].relocation;
                    relocation_mask =  symbol_relocation_strings[i].mask;
                    break;
                }
            }

            if (i == SYMBOL_RELOCATION_STRINGS_COUNT)
                goto next_reloc;
            if (sscanf(offset_str, "%" FMT_SIZE "x", &offset) != 1 &&
                sscanf(offset_str, "%" FMT_SIZE "u", &offset) != 1)
                
                goto next_reloc;
            if (offset + 4 > symbol->size)
                goto next_reloc;
            if (offset >= symbol->offset && mask != NULL) {
                mask[offset - symbol->offset + 0] &= relocation_mask[0];
                mask[offset - symbol->offset + 1] &= relocation_mask[1];
                mask[offset - symbol->offset + 2] &= relocation_mask[2];
                mask[offset - symbol->offset + 3] &= relocation_mask[3];
            }

            if (symbol_str != NULL) {
                relocation = Symbol_AddRelocation(
                    symbol, symbol_str, type, offset);
                if (relocation == NULL)
                    goto exit_error;
            }

next_reloc:
            xml_reloc = mxmlFindElement(
                xml_reloc, xml_symbol, "reloc", NULL, NULL, MXML_NO_DESCEND);
        }
            
        symbol->offset += symbol->data_size;

next_symbol:
        xml_symbol = mxmlFindElement(
            xml_symbol, xml_symbols, "symbol", NULL, NULL, MXML_NO_DESCEND);
    }

    result = true;
exit_error:
    mxmlDelete(xml_tree);
    return result;
}

symbol_t *Symbol_GetSymbolSize(symbol_index_t index) {    
    assert(symbol_globals != NULL);
    assert(index < symbol_count);
    
    if (symbol_count == 0)
        return NULL;
    
    if (symbol_size_index == NULL) {
        symbol_index_t i;
        
        symbol_size_index =
            malloc(sizeof(symbol_size_index_entry_t) * symbol_count);
        
        /* I suppose we could do a linear search here... */
        if (symbol_size_index == NULL)
            return NULL;
            
        for (i = 0; i < symbol_count; i++) {
            symbol_size_index[i].data_size = Symbol_GetSymbol(i)->data_size;
            symbol_size_index[i].index = i;
        }
            
        qsort(
            symbol_size_index, symbol_count,
            sizeof(symbol_size_index_entry_t), &Symbol_CompareSize);
    }
    
    assert(symbol_size_index != NULL);
    
    return Symbol_GetSymbol(symbol_size_index[index].index);
}

static symbol_t *Symbol_AllocSymbol(const char *name, size_t name_length) {
    char *name_alloc;
    symbol_t *symbol;
    
    if (symbol_globals == NULL) {
        symbol_globals = malloc(
            SYMBOL_LIST_INITIAL_CAPACITY * sizeof(symbol_t));
        symbol_globals_end = symbol_globals + SYMBOL_LIST_INITIAL_CAPACITY;
        symbol_globals_free = symbol_globals;
    } else if (symbol_globals_end == symbol_globals_free) {
        symbol_t *temp;

        temp = realloc(
            symbol_globals,
            (symbol_globals_end - symbol_globals) * 2 * sizeof(symbol_t));
        
        if (!temp)
            return NULL;
        symbol_globals_end = temp + (symbol_globals_end - symbol_globals) * 2;
        symbol_globals_free = temp + (symbol_globals_free - symbol_globals);
        symbol_globals = temp;

    }

    assert(symbol_globals != NULL);
    assert(symbol_globals_end > symbol_globals);
    assert(symbol_globals_free >= symbol_globals);
    assert(symbol_globals_free < symbol_globals_end);
        
    symbol = symbol_globals_free;

    name_alloc = malloc(name_length + 1);

    if (name_alloc != NULL) {
        if (symbol_alphabetical_index != NULL) {
            free(symbol_alphabetical_index);
            symbol_alphabetical_index = NULL;
        }
        if (symbol_size_index != NULL) {
            free(symbol_size_index);
            symbol_size_index = NULL;
        }
        symbol_globals_free++;
        symbol_count++;
        strncpy(name_alloc, name, name_length + 1);
        symbol->name = name_alloc;
        symbol->relocation = NULL;
        symbol->index = symbol - symbol_globals;
        symbol->debugging = false;
    } else {
        symbol = NULL;
    }

    return symbol;
}

static symbol_relocation_t *Symbol_AddRelocation(
        symbol_t *symbol, const char *target,
        unsigned char type, size_t offset) {
    char *name_alloc;
    symbol_relocation_t *relocation;
    
    assert(symbol);
    
    relocation = malloc(sizeof(symbol_relocation_t));
    
    if (relocation != NULL) {
        assert(target != NULL);
        name_alloc = malloc(strlen(target) + 1);
        if (name_alloc != NULL) {
            strncpy(name_alloc, target, strlen(target) + 1);
            relocation->symbol = name_alloc;
            relocation->type = type;
            relocation->offset = offset;
            relocation->next = symbol->relocation;
            symbol->relocation = relocation;
        } else {
            relocation = NULL;
        }
    }
    
    return relocation;
}

static int Symbol_CompareSize(const void *left_ptr, const void *right_ptr) {
    const symbol_size_index_entry_t *left, *right;
    
    left = (const symbol_size_index_entry_t *)left_ptr;
    right = (const symbol_size_index_entry_t *)right_ptr;

    return left->data_size - right->data_size;
}

static int Symbol_CompareName(const void *left_ptr, const void *right_ptr) {
    const symbol_alphabetical_index_entry_t *left, *right;
    
    left = (const symbol_alphabetical_index_entry_t *)left_ptr;
    right = (const symbol_alphabetical_index_entry_t *)right_ptr;
    
    if (left->name == NULL && right->name == NULL)
        return 0;
    else if (left->name == NULL)
        return -1;
    else if (right->name == NULL)
        return 1;
    else
        return strcmp(left->name, right->name);
}

symbol_alphabetical_index_t Symbol_SearchSymbol(const char *name) {
    symbol_alphabetical_index_entry_t ref, *index_ptr;
    
    assert(name != NULL);
    
    if (symbol_count == 0)
        return SYMBOL_NULL;
    
    if (symbol_alphabetical_index == NULL) {
        symbol_index_t i;
        
        symbol_alphabetical_index =
            malloc(sizeof(symbol_alphabetical_index_entry_t) * symbol_count);
        
        /* I suppose we could do a linear search here... */
        if (symbol_alphabetical_index == NULL)
            return SYMBOL_NULL;
            
        for (i = 0; i < symbol_count; i++) {
            symbol_alphabetical_index[i].name = Symbol_GetSymbol(i)->name;
            symbol_alphabetical_index[i].index = i;
        }
            
        qsort(
            symbol_alphabetical_index, symbol_count,
            sizeof(symbol_alphabetical_index_entry_t), &Symbol_CompareName);
    }
    
    assert(symbol_alphabetical_index != NULL);
    
    ref.name = name;
    
    index_ptr = bsearch(
        &ref, symbol_alphabetical_index, symbol_count,
        sizeof(symbol_alphabetical_index_entry_t), &Symbol_CompareName);
    
    if (index_ptr == NULL)
        return SYMBOL_NULL;
    else {
        symbol_index_t index;
        
        assert(strcmp(index_ptr->name, name) == 0);
        index = index_ptr - symbol_alphabetical_index;
        
        while (index > 0 &&
               strcmp(symbol_alphabetical_index[index - 1].name, name) == 0)
            index--;
            
        return index;
    }
}

symbol_t *Symbol_GetSymbolAlphabetical(symbol_alphabetical_index_t index) {
    assert(symbol_globals != NULL);
    assert(symbol_alphabetical_index != NULL);
    
    return &symbol_globals[symbol_alphabetical_index[index].index];
}