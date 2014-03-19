/* <bslug/bslug.h>
 *  by Alex Chadwick
 */
 
#ifndef BSLUG_BSLUG_H_
#define BSLUG_BSLUG_H_

#include <stddef.h>
#include <stdint.h>

#define BSLUG_SECTION(x) __attribute__((__section__ (".bslug." x)))

typedef enum bslug_loader_entry_type_t {
    BSLUG_LOADER_ENTRY_FUNCTION,
    BSLUG_LOADER_ENTRY_LOAD
} bslug_loader_entry_type_t;

typedef struct bslug_loader_entry_t {
    bslug_loader_entry_type_t type;
    union {
        struct {
            const char *name;
            const void *target;
        } function;
        struct {
            uint32_t address;
            const void *data;
            size_t size;
        } load;
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
                .target = (replace_func) \
            } \
        } \
    }

#define BSLUG_LOAD(load_address, data_symb, data_size) \
    extern const bslug_loader_entry_t bslug_load_ ## data_symb \
        BSLUG_SECTION("load"); \
    extern typeof(data_symb) data_symb BSLUG_SECTION("data." #data_symb); \
    const bslug_loader_entry_t bslug_load_ ## data_symb = { \
        .type = BSLUG_LOADER_ENTRY_LOAD, \
        .data = { \
            .load = { \
                .address = (load_address), \
                .data = data_symb, \
                .size = (data_size) \
            } \
        } \
    }

#define BSLUG_META(id, value) \
    extern const char bslug_meta_ ## id [] BSLUG_SECTION("meta"); \
    const char bslug_meta_ ## id [] = #id "=" value

#define BSLUG_MODULE_GAME(x)    BSLUG_META(game, x)
#define BSLUG_MODULE_NAME(x)    BSLUG_META(name, x)
#define BSLUG_MODULE_AUTHOR(x)  BSLUG_META(author, x)
#define BSLUG_MODULE_VERSION(x) BSLUG_META(version, x)
#define BSLUG_MODULE_LICENSE(x) BSLUG_META(license, x)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* BSLUG_BSLUG_H_ */