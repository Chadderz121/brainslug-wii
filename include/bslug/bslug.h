/* <bslug/bslug.h>
 *  by Alex Chadwick
 */
 
#ifndef BRAINSLUG_BSLUG_H_
#define BRAINSLUG_BSLUG_H_

#include <stddef.h>
#include <stdint.h>

#define BSLUG_VERSION(maj, min, rev) (((maj) << 24) | ((min) << 16) | (rev))

#define BSLUG_LIB_VERSION_MAJOR ((uint8_t)0)
#define BSLUG_LIB_VERSION_MINOR ((uint8_t)1)
#define BSLUG_LIB_VERSION_REVISION ((uint16_t)0)
#define BSLUG_LIB_VERSION BSLUG_VERSION(BSLUG_LIB_VERSION_MAJOR, \
                                        BSLUG_LIB_VERSION_MINOR, \
										BSLUG_LIB_VERSION_REVISION)

enum bslug_loader_entry_type_t {
    BSLUG_LOADER_ENTRY_FUNCTION,
	BSLUG_LOADER_ENTRY_LOAD
};

struct bslug_loader_entry_t {
    enum bslug_loader_entry_type_t type;
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
};

#define BSLUG_REPLACE(original_func, replace_func) \
    extern const struct bslug_loader_entry_t bslug_load_ ## original_func \
        __attribute__((__section__ (".bslug.load"))); \
    const struct bslug_loader_entry_t bslug_load_ ## original_func = { \
        .type = BSLUG_LOADER_ENTRY_FUNCTION, \
        .data = { \
            .function = { \
                .name = #original_func, \
                .target = (replace_func) \
            } \
        } \
    }

#define BSLUG_LOAD(load_address, data_symb, data_size) \
    extern const struct bslug_loader_entry_t bslug_load_ ## data_symb \
        __attribute__((__section__ (".bslug.load"))); \
    extern typeof(data_symb) data_symb \
        __attribute__((__section__ (".bslug.data." #data_symb))); \
    const struct bslug_loader_entry_t bslug_load_ ## data_symb = { \
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
	extern const char bslug_meta_ ## id [] \
        __attribute__((__section__ (".bslug.meta"))); \
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

#endif /* BRAINSLUG_BSLUG_H_ */