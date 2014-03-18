/* event.h
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
 
/* A simple implementation of the 'event' synchronisation primitive.
 * Threads can wait for an event, at which point they block until it is
 * triggered.
 * An event can be triggered by any thread, causing all waiting threads to be
 * released.
 */
 
#ifndef EVENT_H_
#define EVENT_H_

#include <assert.h>
#include <ogc/semaphore.h>
#include <stdbool.h>

typedef struct {
    bool triggered; /* fast lockless variable for already triggered events */
    sem_t sem;
} event_t;

static bool Event_Init(event_t *event);
static bool Event_Destroy(event_t *event);
static bool Event_Wait(event_t *event);
static bool Event_Trigger(event_t *event);
static bool Event_Reset(event_t *event);

static inline bool Event_Init(event_t *event) {
    assert(event);
    event->triggered = false;
    return LWP_SemInit(&event->sem, 0, 1) == 0;
}

static inline bool Event_Destroy(event_t *event) {
    int ret;
    
    assert(event);
    assert(event->sem != LWP_SEM_NULL);
    ret = LWP_SemDestroy(event->sem);
#ifndef NDEBUG
    event->sem = LWP_SEM_NULL;
#endif
    return ret == 0;
}

static inline bool Event_Wait(event_t *event) {
    int ret;
    
    assert(event);
    assert(event->sem != LWP_SEM_NULL);
    if (event->triggered) {
        return true;
    } else {
        ret = LWP_SemWait(event->sem);
        if (ret)
            return false;
        return LWP_SemPost(event->sem) == 0;
    }
}

static inline bool Event_Trigger(event_t *event) {
    assert(event);
    assert(event->sem != LWP_SEM_NULL);
    event->triggered = true;
    return LWP_SemPost(event->sem) == 0;
}

static inline bool Event_Reset(event_t *event) {
    assert(event);
    assert(event->sem != LWP_SEM_NULL);
    event->triggered = false;
    return LWP_SemWait(event->sem) == 0;
}

#endif /* EVENT_H_ */
