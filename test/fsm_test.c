/* fsm_test.c
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


#include "../src/search/symbol.h"

#define Symbol_GetSymbol(index) (&fsm_test_symbol[index])
#define Symbol_GetSymbolSize(index) (&fsm_test_symbol[index])

symbol_t fsm_test_symbol[4];

#include "../src/search/fsm.c"
 
#include "fsm_test.h"

#include <stdio.h>
#include <stdint.h>

static void FSMTest_Print(const fsm_t *fsm) {
    fsm_node_t *nodes[fsm->node_count];
    unsigned int i, j, found;
    
    assert(fsm);
    assert(fsm->initial != NULL);
        
    if (fsm->initial != NULL) {
        for (i = 0; i < fsm->node_count; i++)
            nodes[i] = NULL;    
    
        assert(fsm->initial->index < fsm->node_count);
        nodes[fsm->initial->index] = fsm->initial;
    
        found = 1;
        while (found < fsm->node_count) {
#ifndef NDEBUG
            unsigned int last_found = found;
#endif
            for (i = 0; i < fsm->node_count; i++) {
                if (nodes[i] != NULL) {
                    if (nodes[i]->symbol == SYMBOL_NULL) {
                        /* transitional node */                    
                        for (j = 0; j < 16; j++) {
                            fsm_node_t *node;
                            
                            node = nodes[i]->payload.transition[j];
                        
                            if (node != NULL) {
                                assert(node->index < fsm->node_count);
                                
                                if (nodes[node->index] == NULL) {
                                    found++;
                                    nodes[node->index] = node;
                                }
                            }
                        }
                    } else {
                        /* epsilon node */
                        fsm_node_t *node;
                        
                        node = nodes[i]->payload.next;
                        
                        if (node != NULL) {
                            assert(node->index < fsm->node_count);
                            
                            if (nodes[node->index] == NULL) {
                                found++;
                                nodes[node->index] = node;
                            }
                        }
                    }
                }
            }
            
            assert(last_found < found);
        }
    }
    
    printf(
        "== fsm ==\n"
        "%u nodes, %u is starting node\n",
        fsm->node_count, fsm->initial->index);
    
    for (i = 0; i < fsm->node_count; i++) {
        if (nodes[i]->symbol == SYMBOL_NULL) {
            printf(
                "node %u: ["
                "%d, %d, %d, %d, %d, %d, %d, %d, "
                "%d, %d, %d, %d, %d, %d, %d, %d]\n",
                i,
                nodes[i]->payload.transition[0] ?
                    (int)nodes[i]->payload.transition[0]->index : -1,
                nodes[i]->payload.transition[1] ?
                    (int)nodes[i]->payload.transition[1]->index : -1,
                nodes[i]->payload.transition[2] ?
                    (int)nodes[i]->payload.transition[2]->index : -1,
                nodes[i]->payload.transition[3] ?
                    (int)nodes[i]->payload.transition[3]->index : -1,
                nodes[i]->payload.transition[4] ?
                    (int)nodes[i]->payload.transition[4]->index : -1,
                nodes[i]->payload.transition[5] ?
                    (int)nodes[i]->payload.transition[5]->index : -1,
                nodes[i]->payload.transition[6] ?
                    (int)nodes[i]->payload.transition[6]->index : -1,
                nodes[i]->payload.transition[7] ?
                    (int)nodes[i]->payload.transition[7]->index : -1,
                nodes[i]->payload.transition[8] ?
                    (int)nodes[i]->payload.transition[8]->index : -1,
                nodes[i]->payload.transition[9] ?
                    (int)nodes[i]->payload.transition[9]->index : -1,
                nodes[i]->payload.transition[10] ?
                    (int)nodes[i]->payload.transition[10]->index : -1,
                nodes[i]->payload.transition[11] ?
                    (int)nodes[i]->payload.transition[11]->index : -1,
                nodes[i]->payload.transition[12] ?
                    (int)nodes[i]->payload.transition[12]->index : -1,
                nodes[i]->payload.transition[13] ?
                    (int)nodes[i]->payload.transition[13]->index : -1,
                nodes[i]->payload.transition[14] ?
                    (int)nodes[i]->payload.transition[14]->index : -1,
                nodes[i]->payload.transition[15] ?
                    (int)nodes[i]->payload.transition[15]->index : -1);
        } else {
            printf(
                "node %u: [%d]\n",
                i,
                nodes[i]->payload.next ? nodes[i]->payload.next->index : -1);
        }
    }
}

int FSMTest_Create0(void) {
    fsm_t *fsm;
    symbol_t *sym;
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };

    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
        
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create1(void) {
    fsm_t *fsm;
    symbol_t *sym;
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x01 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
        
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create2(void) {
    fsm_t *fsm;
    symbol_t *sym;
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x00 };
    uint8_t mask[] = { 0x00, 0xff, 0xff, 0x00 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
        
    fsm = FSM_Create(0);
    
    if (fsm) { 
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create3(void) {
    fsm_t *fsm;
    symbol_t *sym;
    uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
    uint8_t mask[] = { 0x00, 0xff, 0xff, 0x00 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    
    fsm = FSM_Create(0);
    
    if (fsm) { 
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create4(void) {
    fsm_t *fsm;
    symbol_t *sym;
    uint8_t data[] = { 0x10, 0x10, 0x10, 0x10 };
    uint8_t mask[] = { 0xf0, 0xf0, 0xf0, 0xf0 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    
    fsm = FSM_Create(0);
    
    if (fsm) { 
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Merge0(void) {
    fsm_t *fsm1, *fsm2, *fsm3 = NULL;
    symbol_t *sym;
    uint8_t data1[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t mask1[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t data2[] = { 0x05, 0x06, 0x07, 0x08 };
    uint8_t mask2[] = { 0xff, 0xff, 0xff, 0xff };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data1;
    sym->mask = mask1;
    sym->data_size = sizeof(data1);
    
    sym = Symbol_GetSymbol(1);
    sym->index = 1;
    sym->data = data2;
    sym->mask = mask2;
    sym->data_size = sizeof(data2);
    
    fsm1 = FSM_Create(0);
    fsm2 = FSM_Create(1);
    
    if (fsm1 && fsm2) { 
        fsm3 = FSM_Merge(fsm1, fsm2);
        
        if (fsm3) {
            FSM_Free(fsm3);
        }
    }
    
    if (fsm1)
        FSM_Free(fsm1);
    if (fsm2)
        FSM_Free(fsm2);

    return fsm3 == NULL;
}

int FSMTest_Merge1(void) {
    fsm_t *fsm1, *fsm2, *fsm3 = NULL;
    symbol_t *sym;
    uint8_t data1[] = { 0x00, 0x01, 0x00, 0x00 };
    uint8_t mask1[] = { 0x00, 0xff, 0xff, 0x00 };
    uint8_t data2[] = { 0x01, 0x00, 0x00, 0x00 };
    uint8_t mask2[] = { 0xff, 0x00, 0x00, 0xff };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data1;
    sym->mask = mask1;
    sym->data_size = sizeof(data1);
    
    sym = Symbol_GetSymbol(1);
    sym->index = 1;
    sym->data = data2;
    sym->mask = mask2;
    sym->data_size = sizeof(data2);
    
    fsm1 = FSM_Create(0);
    fsm2 = FSM_Create(1);
    
    if (fsm1 && fsm2) { 
        fsm3 = FSM_Merge(fsm1, fsm2);
        
        if (fsm3) {
            FSM_Free(fsm3);
        }
    }
    
    if (fsm1)
        FSM_Free(fsm1);
    if (fsm2)
        FSM_Free(fsm2);

    return fsm3 == NULL;
}

void FSMTest_SymbolDetect(const symbol_index_t symbol, uint8_t *address) {
    symbol_t *sym;
    uint8_t **results;
    
    sym = Symbol_GetSymbol(symbol);
    results = (uint8_t **)sym->name;
    results[sym->size++] = address;
}

int FSMTest_Run0(void) {
    fsm_t *fsm;
    symbol_t *sym;
    const uint8_t *results[1];
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t test[] = { 0x00, 0x01, 0x02, 0x03 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    sym->offset = 4;
    sym->name = (const char *)results;
    sym->size = 0;
    
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Run(fsm, test, sizeof(test), FSMTest_SymbolDetect);
    
        FSM_Free(fsm);
    }
        
    if (Symbol_GetSymbol(0)->size != 1)
        return 101;
    if (results[0] != &test[0])
        return 102;

    return fsm == NULL;
}

int FSMTest_Run1(void) {
    fsm_t *fsm;
    symbol_t *sym;
    const uint8_t *results[2];
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t test[] = { 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    sym->offset = 4;
    sym->name = (const char *)results;
    sym->size = 0;
    
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Run(fsm, test, sizeof(test), FSMTest_SymbolDetect);
    
        FSM_Free(fsm);
    }
        
    if (Symbol_GetSymbol(0)->size != 2)
        return 101;
    if (results[0] != &test[1])
        return 102;
    if (results[1] != &test[5])
        return 103;

    return fsm == NULL;
}

int FSMTest_Run2(void) {
    fsm_t *fsm;
    symbol_t *sym;
    const uint8_t *results[2];
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x01 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t test[] = { 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    sym->offset = 4;
    sym->name = (const char *)results;
    sym->size = 0;
    
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Run(fsm, test, sizeof(test), FSMTest_SymbolDetect);
    
        FSM_Free(fsm);
    }
        
    if (Symbol_GetSymbol(0)->size != 2)
        return 101;
    if (results[0] != &test[1])
        return 102;
    if (results[1] != &test[3])
        return 103;

    return fsm == NULL;
}

int FSMTest_Run3(void) {
    fsm_t *fsm;
    symbol_t *sym;
    const uint8_t *results[3];
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x00 };
    uint8_t mask[] = { 0x00, 0xff, 0xff, 0x00 };
    uint8_t test[] = { 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x03 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data;
    sym->mask = mask;
    sym->data_size = sizeof(data);
    sym->offset = 4;
    sym->name = (const char *)results;
    sym->size = 0;
    
    fsm = FSM_Create(0);
    
    if (fsm) {
        FSM_Run(fsm, test, sizeof(test), FSMTest_SymbolDetect);
    
        FSM_Free(fsm);
    }
        
    if (Symbol_GetSymbol(0)->size != 3)
        return 101;
    if (results[0] != &test[1])
        return 102;
    if (results[1] != &test[3])
        return 103;
    if (results[2] != &test[5])
        return 104;


    return fsm == NULL;
}

int FSMTest_Run4(void) {
    fsm_t *fsm1, *fsm2, *fsm3 = NULL;
    symbol_t *sym;
    const uint8_t *results1[3];
    const uint8_t *results2[3];
    uint8_t data1[] = { 0x00, 0x01, 0x00, 0x00 };
    uint8_t mask1[] = { 0x00, 0xff, 0xff, 0x00 };
    uint8_t data2[] = { 0x01, 0x00, 0x00, 0x00 };
    uint8_t mask2[] = { 0xff, 0x00, 0x00, 0xff };
    uint8_t test[] = { 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x03 };
    
    sym = Symbol_GetSymbol(0);
    sym->index = 0;
    sym->data = data1;
    sym->mask = mask1;
    sym->data_size = sizeof(data1);
    sym->offset = 4;
    sym->name = (const char *)results1;
    sym->size = 0;
    
    sym = Symbol_GetSymbol(1);
    sym->index = 1;
    sym->data = data2;
    sym->mask = mask2;
    sym->data_size = sizeof(data2);
    sym->offset = 4;
    sym->name = (const char *)results2;
    sym->size = 0;
    
    fsm1 = FSM_Create(0);
    fsm2 = FSM_Create(1);
    
    if (fsm1 && fsm2) { 
        fsm3 = FSM_Merge(fsm1, fsm2);
        
        if (fsm3) {
            FSM_Run(fsm3, test, sizeof(test), FSMTest_SymbolDetect);
        
            FSM_Free(fsm3);
        }
    }
    
    if (fsm1)
        FSM_Free(fsm1);
    if (fsm2)
        FSM_Free(fsm2);
        
    if (Symbol_GetSymbol(0)->size != 3)
        return 101;
    if (results1[0] != &test[0])
        return 102;
    if (results1[1] != &test[3])
        return 103;
    if (results1[2] != &test[5])
        return 104;
    if (Symbol_GetSymbol(1)->size != 2)
        return 105;
    if (results2[0] != &test[0])
        return 106;
    if (results2[1] != &test[4])
        return 107;
        
    return fsm3 == NULL;
}
