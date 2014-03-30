/* module.h
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

#ifndef MODULE_H_
#define MODULE_H_

#include <stdbool.h>
#include <stddef.h>

#include "library/event.h"

typedef struct {
    const char *path;
    const char *game;
    const char *name;
    const char *author;
    const char *version;
    const char *license;
    size_t size;
    size_t entries_count;
} module_metadata_t;

extern event_t module_event_list_loaded;
extern event_t module_event_complete;
extern bool module_has_error;
/* whether or not to delay loading for debug messages. */
extern bool module_has_info;

extern size_t module_list_size;
extern module_metadata_t **module_list;
extern size_t module_list_count;

bool Module_Init(void);
bool Module_RunBackground(void);

#endif /* MODULE_H_ */
