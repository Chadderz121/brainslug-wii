/* OSThread.h
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

/* definitions of symbols inferred to exist in the OSThread header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_OSTHREAD_H_
#define _RVL_OSTHREAD_H_

#include <stdint.h>

typedef struct {
  uint32_t gpr[32]; /*!< 0x0 from OSLoadContext */
  uint32_t cr; /*!< 0x80 from OSLoadContext */
  uint32_t lr; /*!< 0x84 from OSLoadContext */
  uint32_t ctr; /*!< 0x88 from OSLoadContext */
  uint32_t xer; /*!< 0x8c from OSLoadContext */
  double fpr[32]; /*!< 0x90 from guesswork */
  char _unknown190[0x198 - 0x190];
  uint32_t srr0; /*!< 0x198 from OSLoadContext */
  uint32_t srr1; /*!< 0x19c from OSLoadContext */
  char _unknown1a0[0x1a2 - 0x1a0];
  struct {
    unsigned : 1; /* 15 */
    unsigned restore_volatile_gpr : 1; /*!< 14 from OSCreateThread; whether or not to restore volatile gprs */
    unsigned : 14; /* 0-13 */
  } ctx_flags; /*!< 0x1a2 from OSLoadContext */
  uint32_t gqr[8]; /*!< 0x1a4 from OSLoadContext */
  char _unknown1c4[0x2ca - 0x1c4];
  int16_t statel            /*!< 0x2c8 from OSIsThreadTerminated */
  int16_t param_r9;         /*!< 0x2ca from OSCreateThread */
  int32_t param_stack_size; /*!< 0x2cc from OSCreateThread */
  int32_t param_priorty;   /*!< 0x2d0 from OSCreateThread */
  int32_t priority;   /*!< 0x2d4 from OSCreateThread */
  char _unknown2d8[0x30c - 0x2d8];
  int32_t errno; /*!< 0x30c Guess from networking */
  char _unknown310[0x318 - 0x310];
} OSThread_t;

OSThread_t *OSGetCurrentThread(void);

#endif /* _RVL_OSTHREAD_H_ */