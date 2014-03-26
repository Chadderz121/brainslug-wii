/* fsm.h
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
 
#ifndef FSM_H_
#define FSM_H_

#include <stddef.h>
#include <stdint.h>

#include "symbol.h"

typedef struct fsm_t fsm_t;

/* function to run on a symbol match. */
typedef void (*fsm_match_t)(symbol_index_t symbol, uint8_t *addr);

fsm_t *FSM_Create(symbol_index_t symbol);
fsm_t *FSM_Merge(const fsm_t *left, const fsm_t *right);
void FSM_Free(fsm_t *fsm);
void FSM_Run(
    const fsm_t *fsm, uint8_t *data,
    size_t length, fsm_match_t match_fn);

#endif /* FSM_H_ */
