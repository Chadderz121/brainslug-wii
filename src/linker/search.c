/* search.c
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
 
#include "search.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

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
} relocaction_t;

/* To be more efficient we construct a finite state machine (FSM) to do the
 * search. This is basically a flow chart that tells us what to do at each step.
 * The idea is constructing this is expensive and slow, but means we can search
 * all symbols in a single pass! The FSM is represented as a series of nodes 
 * with transitions between them. At any point, only one transition should be
 * valid, based only on the next byte in the ram. */

struct symbol_t;
struct relocation_instance_t;
struct parser_node_t;
struct parser_transition_t;

typedef struct symbol_t {
    const char *name;
    const struct relocation_instance_t *relocations;
} symbol_t;

typedef struct relocation_instance_t {
    const symbol_t *symbol;
    relocation_t type;
    unsigned int offset;
} relocation_instance_t;

typedef struct parser_node_t {
    /* if symbol != NULL, next is valid, otherwise transition is. */
    const symbol_t *symbol;
    union {
        parser_node_t *next;
        struct parser_transition_t *transition;
    } payload;
} parser_node_t;

typedef struct parser_transition_t {
    uint8_t value;
    uint8_t mask;
    parser_node_t *node;
    struct parser_transition_t *next;
} parser_transition_t;

typedef struct {
    /* For efficiency, we want to reduce the number of malloc calls. Therefore,
     * we perform mallocs of large arrays to hold all the above structures, then
     * realloc them into a bigger array if we run out of space. 
     * head is the start of this array
     * free is a pointer to the first unused entry
     * end is the end of the array */
    parser_node_t *node_head;
    parser_node_t *node_free;
    parser_node_t *node_end;
    parser_transition_t *transition_head;
    parser_transition_t *transition_free;
    parser_transition_t *transition_end;
    symbol_t *symbol_head;
    symbol_t *symbol_free;
    symbol_t *symbol_end;
    relocation_instance_t *reloc_head;
    relocation_instance_t *reloc_free;
    relocation_instance_t *reloc_end;
    char *name_head;
    char *name_free;
    char *name_end;
} parser_t;
 
static parser_node_t *Search_AllocNode(parser_t *parser) {
    assert(parser);
    assert(symbol);
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free <= parser->node_end);
    
    if (parser->node_free == parser->node_end) {
        // array is full; realloc
        parser_node_t *new_data;
        int i;
        
        new_data = realloc(
            parser->node_head,
            (parser->node_end - parser->node_head) *
                2 * sizeof(parser_node_t));
                
        if (new_data == NULL) {
            // out of memory, fail
            return NULL;
        }
        
        // sadly, we now have to patch up all pointers
        for (i = 0; 
             i < parser->transition_free - parser->transition_head;
             i++) {
            assert(parser->transition_head[i]->node);
            parser->transition_head[i]->node = new_data + 
                (parser->transition_head[i]->node - parser->node_head);
        }
        
        for (i = 0; 
             i < parser->node_free - parser->node_head;
             i++) {
            if (new_data[i]->symbol != NULL &&
                new_data[i]->payload.next != NULL) {
                
                new_data[i]->next = new_data + 
                    (new_data[i]->payload.next - parser->node_head);
            }
        }
        
        parser->node_free = new_data +
            (parser->node_free - parser->node_head);
        parser->node_end = new_data + 
            (parser->node_end - parser->node_head) * 2;
        parser->node_head = new_data;
    }
    
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free + 1 <= parser->node_end);
    
    return parser->node_free++;
}
static parser_node_t *Search_AllocNodeTransitional(parser_t *parser) {
    parser_node_t *node;
    
    node = Search_AllocNode(parser);
    
    if (node) {
        node->symbol = NULL;
        node->payload.transition = NULL;
    }
    
    return node;
}
static parser_node_t *Search_AllocNodeEpsilon(
        parser_t *parser, const symbol_t *symbol) {
    parser_node_t *node;
    
    assert(symbol);
    
    node = Search_AllocNode(parser);
    
    if (node) {
        node->symbol = symbol;
        node->payload.next = NULL;
    }
    
    return node;
}
 
static parser_transition_t *Search_AllocTransition(
        parser_t *parser, uint8_t value, uint8_t mask,
        parser_node_t *node, parser_node_t *target) {
    assert(parser);
    assert(node);
    assert(node->symbol == NULL);
    assert(target);
    assert(parser->transition_head);
    assert(parser->transition_head < parser->transition_end);
    assert(parser->transition_free >= parser->transition_head);
    assert(parser->transition_free <= parser->transition_end);
    
    if (parser->transition_free == parser->transition_end) {
        // array is full; realloc
        parser_transition_t *new_data;
        int i;
        
        new_data = realloc(
            parser->transition_head,
            (parser->transition_end - parser->transition_head) *
                2 * sizeof(parser_transition_t));
                
        if (new_data == NULL) {
            // out of memory, fail
            return NULL;
        }
        
        // sadly, we now have to patch up all pointers
        for (i = 0; 
             i < parser->transition_free - parser->transition_head;
             i++) {
            if (new_data[i]->next) {
                new_data[i]->next = new_data + 
                    (new_data[i]->next - parser->transition_head);
            }
        }
        
        for (i = 0; 
             i < parser->node_free - parser->node_head;
             i++) {
            if (parser->node_head[i]->symbol == NULL &&
                parser->node_head[i]->payload.transition != NULL) {
                
                parser->node_head[i]->transition = new_data + 
                    (parser->node_head[i]->payload.transition -
                     parser->transition_head);
            }
        }
        
        parser->transition_free = new_data +
            (parser->transition_free - parser->transition_head);
        parser->transition_end = new_data + 
            (parser->transition_end - parser->transition_head) * 2;
        parser->transition_head = new_data;
    }
    
    assert(parser->transition_head);
    assert(parser->transition_head < parser->transition_end);
    assert(parser->transition_free >= parser->transition_head);
    assert(parser->transition_free + 1 <= parser->transition_end);
    
    parser->transition_free->value = value;
    parser->transition_free->mask = mask;
    parser->transition_free->node = target;
    parser->transition_free->next = node->payload.transition;
    node->payload.transition = parser->transition_free;
    
    return parser->transition_free++;
}
 
static symbol_t *Search_AllocSymbol(
        parser_t *parser, const char *name, size_t name_length) {
    assert(parser);
    assert(parser->symbol_head);
    assert(parser->symbol_head < parser->symbol_end);
    assert(parser->symbol_free >= parser->symbol_head);
    assert(parser->symbol_free <= parser->symbol_end);
    
    if (parser->symbol_free == parser->symbol_end) {
        // array is full; realloc
        symbol_t *new_data;
        int i;
        
        new_data = realloc(
            parser->symbol_head,
            (parser->symbol_end - parser->symbol_head) *
                2 * sizeof(symbol_t));
                
        if (new_data == NULL) {
            // out of memory, fail
            return NULL;
        }
        
        // sadly, we now have to patch up all pointers
        for (i = 0; 
             i < parser->transition_free - parser->transition_head;
             i++) {
            parser->transition_head[i]->state = new_data + 
                (parser->transition_head[i]->state - parser->symbol_head);
        }
        
        parser->symbol_free = new_data +
            (parser->symbol_free - parser->symbol_head);
        parser->symbol_end = new_data + 
            (parser->symbol_end - parser->symbol_head) * 2;
        parser->symbol_head = new_data;
    }
    
    assert(parser->symbol_head);
    assert(parser->symbol_head < parser->symbol_end);
    assert(parser->symbol_free >= parser->symbol_head);
    assert(parser->symbol_free + 1 <= parser->symbol_end);
    
    return parser->symbol_free++;
}
 
static parser_node_t *Search_AllocNode(parser_t *parser) {
    assert(parser);
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free <= parser->node_end);
    
    if (parser->node_free == parser->node_end) {
        // array is full; realloc
        parser_node_t *new_data;
        int i;
        
        new_data = realloc(
            parser->node_head,
            (parser->node_end - parser->node_head) *
                2 * sizeof(parser_node_t));
                
        if (new_data == NULL) {
            // out of memory, fail
            return NULL;
        }
        
        // sadly, we now have to patch up all pointers
        for (i = 0; 
             i < parser->transition_free - parser->transition_head;
             i++) {
            parser->transition_head[i]->state = new_data + 
                (parser->transition_head[i]->state - parser->node_head);
        }
        
        parser->node_free = new_data +
            (parser->node_free - parser->node_head);
        parser->node_end = new_data + 
            (parser->node_end - parser->node_head) * 2;
        parser->node_head = new_data;
    }
    
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free + 1 <= parser->node_end);
    
    return parser->node_free++;
}
 
static parser_node_t *Search_AllocNode(parser_t *parser) {
    assert(parser);
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free <= parser->node_end);
    
    if (parser->node_free == parser->node_end) {
        // array is full; realloc
        parser_node_t *new_data;
        int i;
        
        new_data = realloc(
            parser->node_head,
            (parser->node_end - parser->node_head) *
                2 * sizeof(parser_node_t));
                
        if (new_data == NULL) {
            // out of memory, fail
            return NULL;
        }
        
        // sadly, we now have to patch up all pointers
        for (i = 0; 
             i < parser->transition_free - parser->transition_head;
             i++) {
            parser->transition_head[i]->state = new_data + 
                (parser->transition_head[i]->state - parser->node_head);
        }
        
        parser->node_free = new_data +
            (parser->node_free - parser->node_head);
        parser->node_end = new_data + 
            (parser->node_end - parser->node_head) * 2;
        parser->node_head = new_data;
    }
    
    assert(parser->node_head);
    assert(parser->node_head < parser->node_end);
    assert(parser->node_free >= parser->node_head);
    assert(parser->node_free + 1 <= parser->node_end);
    
    return parser->node_free++;
}
