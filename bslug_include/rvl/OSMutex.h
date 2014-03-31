/* OSMutex.h
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

/* definitions of symbols inferred to exist in the OSMutex.h header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_OSMUTEX_H_
#define _RVL_OSMUTEX_H_

#include <stdbool.h>
#include <rvl/OSThread.h>

typedef struct OSMutex_t OSMutex_t;
typedef struct OSCond_t OSCond_t;

/* Helpful observation: OSInitMutex and OSInitCond just zero their parameters.
 * Thus you don't _HAVE_ to init a mutex, you can just zero it. */

static inline void OSInitMutex(OSMutex_t *mutex);
void OSLockMutex(OSMutex_t *mutex);
void OSUnlockMutex(OSMutex_t *mutex);
bool OSTryLockMutex(OSMutex_t *mutex);

static inline void OSInitCond(OSCond_t *cond);
/* mutex cannot be NULL */
void OSWaitCond(OSCond_t *cond, OSMutex_t *mutex);
static inline void OSSignalCond(OSCond_t *cond);

struct OSMutex_t {
    OSThreadQueue_t queue;
    OSThread_t *lock_holder;
    int lock_count;
    OSMutex_t *next_held_next;
    OSMutex_t *next_held_prev;
};

static inline void OSInitMutex(OSMutex_t *mutex) {
    OSInitThreadQueue(&mutex->queue);
    mutex->lock_holder = NULL;
    mutex->lock_count = 0;
}

struct OSCond_t {
    OSThreadQueue_t queue;
};

static inline void OSInitCond(OSCond_t *cond) {
    OSInitThreadQueue(&cond->queue);    
}
static inline void OSSignalCond(OSCond_t *cond) {
    OSWakeupThread(&cond->queue);
}

#endif /* _RVL_OSMUTEX_H_ */