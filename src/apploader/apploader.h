/* apploader.h
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

#ifndef APPLOADER_H_
#define APPLOADER_H_

#include <stdbool.h>
#include <stdint.h>

#include "library/event.h"

typedef void (*apploader_game_entry_t)(void);

extern event_t apploader_event_disk_id;
extern event_t apploader_event_complete;
extern apploader_game_entry_t apploader_game_entry_fn;
extern uint8_t *apploader_app0_start;
extern uint8_t *apploader_app0_end;
extern uint8_t *apploader_app1_start;
extern uint8_t *apploader_app1_end;

bool Apploader_Init(void);
bool Apploader_RunBackground(void);

#endif /* APPLOADER_H_ */
