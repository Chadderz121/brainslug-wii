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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct OSThread_t OSThread_t;
typedef struct OSThreadQueue_t OSThreadQueue_t;
typedef struct OSMutex_t OSMutex_t;
typedef void *(*OSThreadEntry_t)(void *argument);

#define THREAD_PRIORITY_LOWEST 0
#define THREAD_PRIORITY_HIGHEST 31

static inline void OSInitThreadQueue(OSThreadQueue_t *queue);
OSThread_t *OSGetCurrentThread(void);
bool OSIsThreadTerminated(OSThread_t *thread);
bool OSCreateThread(
    OSThread_t *thread, OSThreadEntry_t entry_point, void *argument,
    void *stack_base, size_t stack_size, int priority, bool detached);
void OSExitThread(void *exit_code);
void OSCancelThread(OSThread_t *thread);
void OSSleepThread(OSThreadQueue_t *queue);
void OSWakeupThread(OSThreadQueue_t *queue);
bool OSSetThreadPriority(OSThread_t *thread, int priority);
static inline int OSGetThreadPriority(OSThread_t *thread);

struct OSThreadQueue_t {
    OSThread_t *_unknown0;
    OSThread_t *_unknown4;
};

static inline void OSInitThreadQueue(OSThreadQueue_t *queue) {
    queue->_unknown0 = NULL;
    queue->_unknown4 = NULL;
}

struct OSThread_t {
  uint32_t gpr[32]; /* 0x0 from OSLoadContext */
  uint32_t cr; /* 0x80 from OSLoadContext */
  uint32_t lr; /* 0x84 from OSLoadContext */
  uint32_t ctr; /* 0x88 from OSLoadContext */
  uint32_t xer; /* 0x8c from OSLoadContext */
  double fpr[32]; /* 0x90 from guesswork */
  char _unknown190[0x198 - 0x190];
  uint32_t srr0; /* 0x198 from OSLoadContext */
  uint32_t srr1; /* 0x19c from OSLoadContext */
  char _unknown1a0[0x1a2 - 0x1a0];
  struct {
    unsigned : 1; /* 15 */
    unsigned restore_volatile_gpr : 1; /* 14 from OSCreateThread; whether or not to restore volatile gprs */
    unsigned : 14; /* 0-13 */
  } ctx_flags; /* 0x1a2 from OSLoadContext */
  uint32_t gqr[8]; /* 0x1a4 from OSLoadContext */
  char _unknown1c4[0x2ca - 0x1c4];
  short state; /* 0x2c8 from OSIsThreadTerminated */
  short param_r9; /* 0x2ca from OSCreateThread */
  int param_stack_size; /* 0x2cc from OSCreateThread */
  int param_priorty; /* 0x2d0 from OSCreateThread */
  int priority; /* 0x2d4 from OSGetThreadPriority */
  void *exit_code; /* 0x2d8 from OSExitThread */
  OSThreadQueue_t *sleep_queue; /* 0x2dc from OSSleepThread */
  char _unknown2e0[0x2e8 - 0x2e0];
  OSThreadQueue_t _unknown2e8; /* 0x2e8 from OSDetachThread */
  OSMutex_t *mutex_waiting; /* 0x2f0 from OSLockMutex */
  OSMutex_t *mutex_held_first; /* 0x2f4 from __OSUnlockAllMutex */
  OSMutex_t *mutex_held_last; /* 0x2f8 from __OSUnlockAllMutex */
  char _unknown2fc[0x30c - 0x2fc];
  int _errno; /* 0x30c Guess from networking */
  char _unknown310[0x318 - 0x310];
};

static inline int OSGetThreadPriority(OSThread_t *thread) {
    return thread->priority;
}

#endif /* _RVL_OSTHREAD_H_ */