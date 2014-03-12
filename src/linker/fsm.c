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

/* This file should ideally avoid Wii specific methods so unit testing can be
 * conducted elsewhere. */
 
#include "fsm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* To be more efficient we construct a finite state machine (FSM) to do the
 * search. This is basically a flow chart that tells us what to do at each step.
 * The idea is constructing this is expensive and slow, but means we can search
 * all symbols in a single pass! The FSM is represented as a series of nodes 
 * with transitions between them. At any point, only one transition should be
 * valid, based only on the next byte in the ram. */

struct fsm_node_t;
struct fsm_transition_t;

typedef struct fsm_node_t {
    unsigned int index;
    /* if symbol != NULL then transition must be. This means it's an epsilon
     * node. */
    const symbol_t *symbol;
    union {
        /* list of transitions given certain characters */
        struct fsm_node_t *transition[16];
        /* default transition if no other applies */
        struct fsm_node_t *next;
    } payload;
} fsm_node_t;

struct fsm_t {
    fsm_node_t *initial;
    unsigned int node_count;
};

typedef struct {
    fsm_node_t *node;
    const fsm_node_t *left;
    const fsm_node_t *right;
    bool processed;
} node_index_t;
 
static fsm_node_t *FSM_AllocNode(fsm_t *fsm) {
    fsm_node_t *node;
    
    assert(fsm);
    
    node = malloc(sizeof(fsm_node_t));
    
    if (node) {
        node->index = fsm->node_count;
        node->symbol = NULL;
        fsm->node_count++;
    }
    
    return node;
}
 
fsm_t *FSM_Create(
        const symbol_t *symbol, const uint8_t *data, 
        const uint8_t *mask, size_t length) {
    fsm_t *fsm = NULL;
    fsm_node_t *last, *node, *fallback, *dummy;
    size_t i;
    unsigned int j;
    
    assert(symbol);
    assert(data);
    assert(mask);
    assert(length > 0);
    
    if (length <= 0)
        return NULL;
    
    fsm = malloc(sizeof(fsm_t));
    
    if (fsm == NULL)
        goto exit_error;
        
    fsm->node_count = 0;
    
    if (mask[0] & 0xf0) {
        dummy = FSM_AllocNode(fsm);
        
        if (dummy == NULL)
            goto exit_error;
    } else
        dummy = NULL;
        
    node = FSM_AllocNode(fsm);
    
    if (node == NULL)
        goto exit_error;
        
    for (j = 0; j < 16; j++) {
        node->payload.transition[j] = dummy;
        if (dummy != NULL)
            dummy->payload.transition[j] = node;
    }
        
    fsm->initial = node;
    last = node;
    fallback = dummy;
        
    for (i = 0; i < length; i++) {
        if (i == 1)
            fallback = fsm->initial;
        else if (i == 0)
            fallback = fsm->initial;
        else
            fallback = fallback->payload.transition[data[i - 1] & 0xf];
    
        node = FSM_AllocNode(fsm);
        
        if (node == NULL)
            goto exit_error;
        
        for (j = 0; j < 16; j++) {
            if ((j & (mask[i] >> 4)) == ((data[i] >> 4) & (mask[i] >> 4))) {
                last->payload.transition[j] = node;
            } else {
                last->payload.transition[j] = fallback->payload.transition[j];
            }
        }
        
        last = node;
        if (i == 0)
            fallback = NULL;
        else
            fallback = fallback->payload.transition[data[i - 1] >> 4];
        
        node = FSM_AllocNode(fsm);
        
        if (node == NULL)
            goto exit_error;
        
        for (j = 0; j < 16; j++) {
            if ((j & (mask[i] & 0xf)) == ((data[i] & 0xf) & (mask[i] & 0xf))) {
                last->payload.transition[j] = node;
            } else if (fallback != NULL) {
                last->payload.transition[j] = fallback->payload.transition[j];
            } else {
                last->payload.transition[j] = fsm->initial;
            }
        }
        
        last = node;
    }
    
    node->symbol = symbol;
    node->payload.next = fallback->payload.transition[data[i - 1] & 0xf];
        
    return fsm;
exit_error:

    if (fsm != NULL) {
        FSM_Free(fsm);
    }

    return NULL;
}
 
static bool FSM_BuildMergeNodeTransitional(
        fsm_t *fsm, const fsm_t *left, const fsm_t *right,
        node_index_t *node_index, fsm_node_t *node,
        const fsm_node_t *left_node, const fsm_node_t *right_node) {
    unsigned int i;
    
    assert(fsm);
    assert(node_index);
    assert(left);
    assert(right);
    assert(node);
    assert(left_node);
    assert(right_node);
        
    /* iterate over both linkage lists */
    for (i = 0; i < 16; i++) {
        unsigned int common_index;
        
        common_index = 
            left_node->payload.transition[i]->index * right->node_count +
            right_node->payload.transition[i]->index;
        
        if (node_index[common_index].node == NULL) {
            assert(!node_index[common_index].processed);
            node_index[common_index].node = FSM_AllocNode(fsm);
        
            if (node_index[common_index].node == NULL)
                return false;
                
            node_index[common_index].left = left_node->payload.transition[i];
            node_index[common_index].right = right_node->payload.transition[i];
        }
        
        node->payload.transition[i] = node_index[common_index].node;
    }
    
    return true;
}
static bool FSM_BuildMergeNodeEpsilon(
        fsm_t *fsm, const fsm_t *left, const fsm_t *right,
        node_index_t *node_index, fsm_node_t *node,
        const fsm_node_t *left_node, const fsm_node_t *right_node) {
    assert(fsm);
    assert(node_index);
    assert(left);
    assert(right);
    assert(node);
    assert(left_node);
    assert(right_node);
    
    assert(left_node->symbol != NULL || right_node->symbol != NULL);
        
    if (left_node->symbol != NULL) {
        unsigned int common_index;
        
        node->symbol = left_node->symbol;
        assert(left_node->payload.next);
        
        common_index = 
            left_node->payload.next->index * right->node_count +
            right_node->index;
        if (node_index[common_index].node == NULL) {
            assert(!node_index[common_index].processed);
            node_index[common_index].node = FSM_AllocNode(fsm);
            node_index[common_index].left = left_node->payload.next;
            node_index[common_index].right = right_node;
        }
        
        if (node_index[common_index].node == NULL)
            return false;
        
        node->payload.next = node_index[common_index].node;
    } else {
        unsigned int common_index;
        
        node->symbol = right_node->symbol;
        assert(right_node->payload.next);
        
        common_index = 
            left_node->index * right->node_count +
            right_node->payload.next->index;
        if (node_index[common_index].node == NULL) {
            assert(!node_index[common_index].processed);
            node_index[common_index].node = FSM_AllocNode(fsm);
            node_index[common_index].left = left_node;
            node_index[common_index].right = right_node->payload.next;
        }
        
        if (node_index[common_index].node == NULL)
            return false;
        
        node->payload.next = node_index[common_index].node;
    }
    
    return true;
}
static bool FSM_BuildMergeNode(
        fsm_t *fsm, const fsm_t *left, const fsm_t *right,
        node_index_t *node_index, fsm_node_t *node,
        const fsm_node_t *left_node, const fsm_node_t *right_node) {
    
    assert(fsm);
    assert(node_index);
    assert(left);
    assert(right);
    assert(node);
    assert(left_node);
    assert(right_node);
    
    if (left_node->symbol != NULL || right_node->symbol != NULL) {
        return FSM_BuildMergeNodeEpsilon(
            fsm, left, right, node_index, node, left_node, right_node);
    } else {
        return FSM_BuildMergeNodeTransitional(
            fsm, left, right, node_index, node, left_node, right_node);
    }
}

fsm_t *FSM_Merge(const fsm_t *left, const fsm_t *right) {
    node_index_t *node_index = NULL;
    fsm_t *fsm = NULL;
    unsigned int processed_nodes, i, common_index;
    
    fsm = malloc(sizeof(fsm_t));
    
    if (fsm == NULL)
        goto exit_error;
    
    fsm->node_count = 0;
    
    node_index = 
        calloc(left->node_count * right->node_count, sizeof(node_index_t));
        
    if (node_index == NULL)
        goto exit_error;
    
    fsm->initial = FSM_AllocNode(fsm);
    
    if (fsm->initial == NULL)
        goto exit_error;
        
    common_index = 
        left->initial->index * right->node_count +
        right->initial->index;
        
    node_index[common_index].node = fsm->initial;
    node_index[common_index].left = left->initial;
    node_index[common_index].right = right->initial;
    
    processed_nodes = 0;
    do {
        for (i = 0; i < left->node_count * right->node_count; i++) {
            if (node_index[i].node != NULL && !node_index[i].processed) {
                if (!FSM_BuildMergeNode(
                        fsm, left, right,
                        node_index, node_index[i].node,
                        node_index[i].left, node_index[i].right))
                    return false;
                node_index[i].processed = true;
                processed_nodes++;
            }
        }   
    } while (processed_nodes != fsm->node_count);
    
    free(node_index);
    
    return fsm;
exit_error:

    if (node_index != NULL) {
        for (i = 0; i < left->node_count * right->node_count; i++) {
            if (node_index[i].node != NULL) {                
                free(node_index[i].node);
            }
        }
        free(node_index);
    }
    if (fsm != NULL)
        free(fsm);

    return NULL;
}

void FSM_Free(fsm_t *fsm) {
    fsm_node_t *nodes[fsm->node_count];
    unsigned int i, j, found;
        
    assert(fsm);
    
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
    
        for (i = 0; i < fsm->node_count; i++) {        
            free(nodes[i]);
        }
    }
    
    free(fsm);
}


void FSM_Run(
        const fsm_t *fsm, const uint8_t *data,
        size_t length, fsm_match_t match_fn) {
    fsm_node_t *state;
    size_t i;
    
    assert(fsm != NULL);
    assert(fsm->initial != NULL);
    assert(data != NULL);
    assert(match_fn != NULL);
    
    state = fsm->initial;
    
    for (i = 0; i < length; i++) {        
        assert(state != NULL);
        
        /* process epsilons */
        while (state->symbol != NULL) {
            match_fn(state->symbol, data + i - state->symbol->offset);
            state = state->payload.next;
            assert(state != NULL);
        }
        
        assert(state->symbol == NULL);
        
        /* process transition */
        state = state->payload.transition[data[i] >> 4];
        assert(state->symbol == NULL);
        state = state->payload.transition[data[i] & 0xf];
    }
}

