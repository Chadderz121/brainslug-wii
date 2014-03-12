/* fsm.c
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

#include "../src/linker/fsm.c"
 
#include "fsm_test.h"

#include <stdio.h>
#include <stdint.h>

#include "../src/linker/search.h"

static void FSMTest_Print(const fsm_t *fsm) {
    fsm_node_t *nodes[fsm->node_count];
    unsigned int i, j, found;
    
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
                    if (nodes[i]->symbol == NULL) {
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
        "== fsm ==\n%u nodes, %u is starting node\n",
        fsm->node_count, fsm->initial->index);
    for (i = 0; i < fsm->node_count; i++) {
        if (nodes[i]->symbol == NULL) {
            printf(
                "node %u: ["
                "%u, %u, %u, %u, %u, %u, %u, %u, "
                "%u, %u, %u, %u, %u, %u, %u, %u]\n",
                i,
                nodes[i]->payload.transition[0]->index,
                nodes[i]->payload.transition[1]->index,
                nodes[i]->payload.transition[2]->index,
                nodes[i]->payload.transition[3]->index,
                nodes[i]->payload.transition[4]->index,
                nodes[i]->payload.transition[5]->index,
                nodes[i]->payload.transition[6]->index,
                nodes[i]->payload.transition[7]->index,
                nodes[i]->payload.transition[8]->index,
                nodes[i]->payload.transition[9]->index,
                nodes[i]->payload.transition[10]->index,
                nodes[i]->payload.transition[11]->index,
                nodes[i]->payload.transition[12]->index,
                nodes[i]->payload.transition[13]->index,
                nodes[i]->payload.transition[14]->index,
                nodes[i]->payload.transition[15]->index);
        } else {
            printf(
                "node %u: [%u]\n",
                i,
                nodes[i]->payload.next->index);
        }
    }
}

int FSMTest_Create0(void) {
    fsm_t *fsm;
    symbol_t symbol;
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    
    fsm = FSM_Create(&symbol, data, mask, sizeof(data));
    
    if (fsm) { 
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create1(void) {
    fsm_t *fsm;
    symbol_t symbol;
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x01 };
    uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff };
    
    fsm = FSM_Create(&symbol, data, mask, sizeof(data));
    
    if (fsm) { 
        FSM_Free(fsm);
    }

    return fsm == NULL;
}

int FSMTest_Create2(void) {
    fsm_t *fsm;
    symbol_t symbol;
    uint8_t data[] = { 0x00, 0x01, 0x00, 0x00 };
    uint8_t mask[] = { 0x00, 0xff, 0xff, 0x00 };
    
    fsm = FSM_Create(&symbol, data, mask, sizeof(data));
    
    if (fsm) { 
        FSMTest_Print(fsm);
        FSM_Free(fsm);
    }

    return fsm == NULL;
}