/* bslug.h
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
 
#ifndef BSLUG_H_
#define BSLUG_H_

#include <stddef.h>
#include <stdint.h>

#define BSLUG_SECTION(x) __attribute__((__section__ (".bslug." x)))

typedef enum bslug_loader_entry_type_t {
    BSLUG_LOADER_ENTRY_FUNCTION,
    BSLUG_LOADER_ENTRY_FUNCTION_MANDATORY,
    BSLUG_LOADER_ENTRY_EXPORT
} bslug_loader_entry_type_t;

typedef struct bslug_loader_entry_t {
    bslug_loader_entry_type_t type;
    union {
        struct {
            const char *name;
            const void *target;
        } function;
        struct {
            const char *name;
            const void *target;
        } export;
    } data;
} bslug_loader_entry_t;

#define BSLUG_REPLACE(original_func, replace_func) \
    extern const bslug_loader_entry_t bslug_load_ ## original_func \
        BSLUG_SECTION("load"); \
    const bslug_loader_entry_t bslug_load_ ## original_func = { \
        .type = BSLUG_LOADER_ENTRY_FUNCTION, \
        .data = { \
            .function = { \
                .name = #original_func, \
                .target = &(replace_func) \
            } \
        } \
    }
#define BSLUG_MUST_REPLACE(original_func, replace_func) \
    extern const bslug_loader_entry_t bslug_load_ ## original_func \
        BSLUG_SECTION("load"); \
    const bslug_loader_entry_t bslug_load_ ## original_func = { \
        .type = BSLUG_LOADER_ENTRY_FUNCTION_MANDATORY, \
        .data = { \
            .function = { \
                .name = #original_func, \
                .target = &(replace_func) \
            } \
        } \
    }
#define BSLUG_EXPORT(symbol) \
    extern const bslug_loader_entry_t bslug_export_ ## symbol \
        BSLUG_SECTION("load"); \
    const bslug_loader_entry_t bslug_export_ ## symbol = { \
        .type = BSLUG_LOADER_ENTRY_EXPORT, \
        .data = { \
            .export = { \
                .name = #symbol, \
                .target = &(symbol) \
            } \
        } \
    }

#define BSLUG_META(id, value) \
    extern const char bslug_meta_ ## id [] BSLUG_SECTION("meta"); \
    const char bslug_meta_ ## id [] = #id "=" value

#define BSLUG_MODULE_GAME(x)    BSLUG_META(game, x)
#define BSLUG_MODULE_NAME(x)    BSLUG_META(name, x); BSLUG_META(bslug, "0.1")
#define BSLUG_MODULE_AUTHOR(x)  BSLUG_META(author, x)
#define BSLUG_MODULE_VERSION(x) BSLUG_META(version, x)
#define BSLUG_MODULE_LICENSE(x) BSLUG_META(license, x)

/* bslug_game_start - first address that is part of the game's executable.
 * bslug_game_end   - address after the end of the game's executable.
 * Added in BSLUG_LIB_VERSION 0.1.1 */
extern uint8_t bslug_game_start[];
extern uint8_t bslug_game_end[];

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* BSLUG_BSLUG_H_ */