/* module.c
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

#include "module.h"

#include <assert.h>
#include <bslug_include/bslug.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libelf.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apploader/apploader.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "main.h"
#include "search/search.h"
#include "threads.h"

typedef struct {
    size_t module;
    const char *name;
    void *address;
    size_t offset;
    char type;
    int addend;
} module_unresolved_relocation_t;

event_t module_event_list_loaded;
event_t module_event_complete;

bool module_has_error;
bool module_has_info;

#define MODULE_LIST_CAPACITY_DEFAULT 16

size_t module_list_size = 0;
module_metadata_t **module_list = NULL;
size_t module_list_count = 0;
static size_t module_list_capacity = 0;

#define MODULE_RELOCATIONS_CAPCITY_DEFAULT 128

static module_unresolved_relocation_t *module_relocations = NULL;
static size_t module_relocations_count = 0;
static size_t module_relocations_capacity = 0;

#define MODULE_ENTRIES_CAPACITY_DEFAULT 128

static bslug_loader_entry_t *module_entries = NULL;
static size_t module_entries_count = 0;
static size_t module_entries_capacity = 0;

static const char module_path[] = "sd:/bslug/modules";

static void *Module_Main(void *arg);
static void *Module_ListAllocate(
    void *list, size_t entry_size, size_t num,
    size_t *capacity, size_t *count, size_t default_capacity);
    
static void Module_ListLoad(void);
static void Module_CheckDirectory(char *path);
static void Module_CheckFile(const char *path);
static void Module_Load(const char *path);
static void Module_LoadElf(const char *path, Elf *elf);
static bool Module_LoadElfSymtab(
    Elf *elf, Elf32_Sym **symtab, size_t *symtab_count, size_t *symtab_strndx);
static module_metadata_t *Module_MetadataRead(
    const char *path, size_t index, Elf *elf, 
    Elf32_Sym *symtab, size_t symtab_count, size_t symtab_strndx);
static bool Module_ElfLoadSection(
    const Elf *elf, Elf_Scn *scn, const Elf32_Shdr *shdr, void *destination);
static void Module_ElfLoadSymbols(
    size_t shndx, const const void *destination, 
    Elf32_Sym *symtab, size_t symtab_count);
static bool Module_ElfLink(
    size_t index, Elf *elf, size_t shndx, void *destination,
    Elf32_Sym *symtab, size_t symtab_count, size_t symtab_strndx,
    bool allow_globals);
static bool Module_ElfLinkOne(
    char type, size_t offset, int addend, void *destination,
    uint32_t symbol_addr);
    
static bool Module_ListLink(uint8_t **space);
static bool Module_LinkModule(size_t index, const char *path, uint8_t **space);
static bool Module_LinkModuleElf(size_t index, Elf *elf, uint8_t **space);

static bool Module_ListLoadSymbols(uint8_t **space);

static bool Module_ListLinkFinal(uint8_t **space);
static bool Module_ListLinkFinalReplaceFunction(
        uint8_t **space, bslug_loader_entry_t *entry);
        
bool Module_Init(void) {
    return
        Event_Init(&module_event_list_loaded) &&
        Event_Init(&module_event_complete);
}

bool Module_RunBackground(void) {
    int ret;
    lwp_t thread;
    
    ret = LWP_CreateThread(
        &thread, &Module_Main,
        NULL, NULL, 0, THREAD_PRIO_IO);
        
    if (ret) {
        errno = ENOMEM;
        return false;
    }
    
    return true;
}

static void *Module_Main(void *arg) {
    uint8_t *space;
    
    Module_ListLoad();
    
    module_list_size += ((-module_list_size) & 0x1f);
    
    Event_Trigger(&module_event_list_loaded);
    
    space = (uint8_t *)0x81800000;
    
    if (!Module_ListLink(&space))
        goto exit_error;
    
    Event_Wait(&apploader_event_complete);
    Event_Wait(&search_event_complete);
    if (search_has_error)
        goto exit_error;
    
    if (!Module_ListLoadSymbols(&space))
        goto exit_error;
    
    if (!Module_ListLinkFinal(&space))
        goto exit_error;
    
    assert(space > (uint8_t *)0x81800000 - module_list_size);
    
    DCFlushRange(space, 0x81800000 - (uint32_t)space);

    Event_Trigger(&module_event_complete);
    
    return NULL;
exit_error:
    printf("Module_Main: exit_error\n");
    module_has_error = true;
    Event_Trigger(&module_event_complete);
    return NULL;
}

static void *Module_ListAllocate(
        void *list, size_t entry_size, size_t num,
        size_t *capacity, size_t *count, size_t default_capacity) {
    void *result;
    
    while (*capacity < *count + num) {
        if (*count == 0) {
            *(void **)list = malloc(entry_size * default_capacity);
            
            if (*(void **)list == NULL)
                return NULL;
            
            *capacity = default_capacity;
        } else {
            void *temp;
            
            temp = realloc(*(void **)list, entry_size * *capacity * 2);
            if (temp == NULL)
                return NULL;
            
            *(void **)list = temp;
            *capacity = *capacity * 2;
        }
    }
    
    result = *(char **)list + entry_size * *count;
    (*count) += num;
    
    return result;
}

static void Module_ListLoad(void) {
    char path[FILENAME_MAX];

    Event_Wait(&main_event_fat_loaded);
    
    assert(sizeof(path) > sizeof(module_path));
    
    strcpy(path, module_path);
    
    Module_CheckDirectory(path);
}

static void Module_CheckDirectory(char *path) {
    DIR *dir;
    
    dir = opendir(path);
    if (dir != NULL) {
        struct dirent *entry;
        
        entry = readdir(dir);
        while (entry != NULL) {
            switch (entry->d_type) {
                case DT_REG: { /* regular file */
                    char *old_path_end;
                    
                    old_path_end = strchr(path, '\0');
                    
                    assert(old_path_end != NULL);
                    
                    /* efficiently concatenate /file_name */
                    strncat(
                        old_path_end, "/",
                        FILENAME_MAX - (old_path_end - path));
                    strncat(
                        old_path_end, entry->d_name,
                        FILENAME_MAX - (old_path_end - path));
                    
                    Module_CheckFile(path);
                    
                    /* reset back to the original path for next file */
                    *old_path_end = '\0';
                    break;
                }
                case DT_DIR: { /* directory */
                    if (strcmp(entry->d_name, ".") == 0 ||
                        strcmp(entry->d_name, "..") == 0)
                        break;
                    
                    Event_Wait(&apploader_event_disk_id);
                    
                    /* load directories with a prefix match on the game name:
                     * e.g. load directory RMC for game RMCP. */
                    if (strncmp(os0->disc.gamename, entry->d_name,
                        strlen(entry->d_name)) == 0) {
                        
                        char *old_path_end;
                        
                        old_path_end = strchr(path, '\0');
                        
                        assert(old_path_end != NULL);
                        
                        /* efficiently concatenate /directory_name */
                        strncat(
                            old_path_end, "/",
                            FILENAME_MAX - (old_path_end - path));
                        strncat(
                            old_path_end, entry->d_name,
                            FILENAME_MAX - (old_path_end - path));
                        
                        Module_CheckDirectory(path);
                        
                        /* reset back to the original path for next file */
                        *old_path_end = '\0';
                    }
                    break;
                }
            }
            entry = readdir(dir);
        }
        closedir(dir);
    }
}

static void Module_CheckFile(const char *path) {
    const char *extension;
    
    /* find the file extension */
    extension = strrchr(path, '.');
    if (extension == NULL)
        extension = strchr(path, '\0');
    else
        extension++;
        
    assert(extension != NULL);
    
    if (strcmp(extension, "mod") == 0 ||
        strcmp(extension, "o") == 0 ||
        strcmp(extension, "a") == 0 ||
        strcmp(extension, "elf") == 0) {
        
        Module_Load(path);
    }
}

static void Module_Load(const char *path) {
    int fd = -1;
    Elf *elf = NULL;
    
    /* check for compile errors */
    if (elf_version(EV_CURRENT) == EV_NONE)
        goto exit_error;
    
    fd = open(path, O_RDONLY, 0);
    
    if (fd == -1)
        goto exit_error;
        
    elf = elf_begin(fd, ELF_C_READ, NULL);
    
    if (elf == NULL)
        goto exit_error;
        
    switch (elf_kind(elf)) {
        case ELF_K_AR:
            /* TODO */
            printf(
                "Warning: Ignoring '%s' - Archives not yet supported.\n", path);
            module_has_info = true;
            goto exit_error;
        case ELF_K_ELF:
            Module_LoadElf(path, elf);
            break;
        default:
            printf(
                "Warning: Ignoring '%s' - Invalid ELF file.\n", path);
            goto exit_error;
    }

exit_error:
    if (elf != NULL)
        elf_end(elf);
    if (fd != -1)
        close(fd);
}

static void Module_LoadElf(const char *path, Elf *elf) {
    Elf_Scn *scn;
    Elf32_Ehdr *ehdr;
    char *ident;
    size_t shstrndx, sz, symtab_count, i, symtab_strndx;
    Elf32_Sym *symtab = NULL;
    module_metadata_t *metadata = NULL;
    module_metadata_t **list_ptr;
    
    assert(elf != NULL);
    assert(elf_kind(elf) == ELF_K_ELF);
    
    ident = elf_getident(elf, &sz);
    
    if (ident == NULL) {
        printf("Warning: Ignoring '%s' - Invalid ELF header.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (sz < 7) {
        printf("Warning: Ignoring '%s' - Invalid ELF header.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ident[4] != ELFCLASS32) {
        printf("Warning: Ignoring '%s' - Not 32 bit ELF.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ident[5] != ELFDATA2MSB) {
        printf("Warning: Ignoring '%s' - Not Big Endian.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ident[6] != EV_CURRENT) {
        printf("Warning: Ignoring '%s' - Unknown ELF version.\n", path);
        module_has_info = true;
        goto exit_error;
    }
        
    ehdr = elf32_getehdr(elf);
    
    if (ehdr == NULL) {
        printf("Warning: Ignoring '%s' - Invalid ELF header\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ehdr->e_type != ET_REL) {
        printf("Warning: Ignoring '%s' - Not relocatable ELF.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ehdr->e_machine != EM_PPC) {
        printf("Warning: Ignoring '%s' - Architecture not EM_PPC.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (ehdr->e_version != EV_CURRENT) {
        printf("Warning: Ignoring '%s' - Unknown ELF version.\n", path);
        module_has_info = true;
        goto exit_error;
    }
        
    if (!Module_LoadElfSymtab(elf, &symtab, &symtab_count, &symtab_strndx)) {
        printf("Warning: Ignoring '%s' - Couldn't parse symtab.\n", path);
        module_has_info = true;
        goto exit_error;
    }
        
    assert(symtab != NULL);
        
    metadata = Module_MetadataRead(
        path, module_list_count, elf, symtab, symtab_count, symtab_strndx);
    
    if (metadata == NULL) /* error reporting done inside method */
        goto exit_error;
    
    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        printf("Warning: Ignoring '%s' - Couldn't find shdrstndx.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    for (i = 0; metadata->game[i] != '\0'; i++) {
        if (metadata->game[i] != '?') {
            Event_Wait(&apploader_event_disk_id);
            if ((i < 4 && metadata->game[i] != os0->disc.gamename[i]) ||
                (i >= 4 && i < 6 &&
                 metadata->game[i] != os0->disc.company[i - 4]) ||
                i >= 6)
                goto exit_error;
        }
    }
    
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
        
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
            
        if ((shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_NOBITS) && 
            (shdr->sh_flags & SHF_ALLOC)) {
            
            const char *name;
                
            name = elf_strptr(elf, shstrndx, shdr->sh_name);
            if (name == NULL)
                continue;
            
            if (strcmp(name, ".bslug.meta") == 0) {
                continue;
            } else if (strcmp(name, ".bslug.load") == 0) {
                metadata->size +=
                    shdr->sh_size / sizeof(bslug_loader_entry_t) * 12;
            } else {
                metadata->size += shdr->sh_size;
                /* add alignment padding to size */
                if (shdr->sh_addralign > 3)
                    /* roundup to multiple of sh_addralign  */
                    metadata->size +=
                        (-metadata->size & (shdr->sh_addralign - 1));
                else
                    /* roundup to multiple of 4 */
                    metadata->size += (-metadata->size & 3);
            }
        }
    }
    
    /* roundup to multiple of 4 */
    metadata->size += (-metadata->size & 3);
    
    list_ptr = Module_ListAllocate(
        &module_list, sizeof(module_metadata_t *), 1, &module_list_capacity,
        &module_list_count, MODULE_LIST_CAPACITY_DEFAULT);
    if (list_ptr == NULL) {
        printf("Warning: Ignoring '%s' - ENOMEM.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    assert(module_list != NULL);
    assert(module_list_count <= module_list_capacity);
    
    *list_ptr = metadata;
    module_list_size += metadata->size;
    /* prevent the data being freed */
    metadata = NULL;
    
exit_error:
    if (metadata != NULL)
        free(metadata);
    if (symtab != NULL)
        free(symtab);
}

static bool Module_LoadElfSymtab(
        Elf *elf, Elf32_Sym **symtab, size_t *symtab_count,
        size_t *symtab_strndx) {
    Elf_Scn *scn;
    bool result = false;

    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
         
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
            
        if (shdr->sh_type == SHT_SYMTAB) {
            size_t sym;
            
            assert (*symtab == NULL);
            *symtab = malloc(shdr->sh_size);
            if (*symtab == NULL)
                continue;
            
            *symtab_count = shdr->sh_size / sizeof(Elf32_Sym);
            *symtab_strndx = shdr->sh_link;

            if (!Module_ElfLoadSection(elf, scn, shdr, *symtab))
                goto exit_error;
            
            for (sym = 0; sym < *symtab_count; sym++)
                (*symtab)[sym].st_other = 0;
            
            
            
            break;
        }
    }
    
    if (*symtab == NULL)
        goto exit_error;
    
    result = true;
exit_error:
    return result;
}

static module_metadata_t *Module_MetadataRead(
        const char *path, size_t index, Elf *elf,
        Elf32_Sym *symtab, size_t symtab_count, size_t symtab_strndx) {
    char *metadata = NULL, *metadata_cur, *metadata_end, *tmp;
    const char *game, *name, *author, *version, *license, *bslug;
    module_metadata_t *ret = NULL;
    Elf_Scn *scn;
    size_t shstrndx, entries_count;
    
    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        printf("Warning: Ignoring '%s' - Couldn't find shstrndx\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    entries_count = 0;
    
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
         
        Elf32_Shdr *shdr;
        const char *name;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
            
        name = elf_strptr(elf, shstrndx, shdr->sh_name);
        if (name == NULL)
            continue;
        
        if (strcmp(name, ".bslug.meta") == 0) {
            if (shdr->sh_size == 0)
                continue;
            
            if (metadata != NULL)
                continue;
            metadata = malloc(shdr->sh_size);
            if (metadata == NULL)
                continue;
                
            if (!Module_ElfLoadSection(elf, scn, shdr, metadata)) {
                printf(
                    "Warning: Ignoring '%s' - Couldn't load .bslug.meta.\n",
                    path);
                module_has_info = true;
                goto exit_error;
            }
            
            Module_ElfLoadSymbols(
                elf_ndxscn(scn), metadata, symtab, symtab_count);
            
            if (!Module_ElfLink(
                    index, elf, elf_ndxscn(scn), metadata,
                    symtab, symtab_count, symtab_strndx, false)) {
                printf(
                    "Warning: Ignoring '%s' - .bslug.meta contains invalid "
                    "relocations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            
            metadata_end = metadata + shdr->sh_size;
            metadata_end[-1] = '\0';
        } else if (strcmp(name, ".bslug.load") == 0) {
            entries_count = shdr->sh_size / sizeof(bslug_loader_entry_t);
        }
    }

    if (metadata == NULL) {
        printf("Warning: Ignoring '%s' - Not a BSLUG module file.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    game = NULL;
    name = NULL;
    author = NULL;
    version = NULL;
    license = NULL;
    bslug = NULL;
    
    for (metadata_cur = metadata;
         metadata_cur < metadata_end;
         metadata_cur = strchr(metadata_cur, '\0') + 1) {
         
        char *eq;
         
        assert(metadata_cur >= metadata && metadata_cur < metadata_end);
        
        if (*metadata_cur == '\0')
            continue;
        
        eq = strchr(metadata_cur, '=');
        if (eq == NULL)
            continue;
        
        if (strncmp(metadata_cur, "game", eq - metadata_cur) == 0) {
            if (game != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_GAME "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            game = eq + 1;
        } else if (strncmp(metadata_cur, "name", eq - metadata_cur) == 0) {
            if (name != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_NAME "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            name = eq + 1;
        } else if (strncmp(metadata_cur, "author", eq - metadata_cur) == 0) {
            if (author != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_AUTHOR "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            author = eq + 1;
        } else if (strncmp(metadata_cur, "version", eq - metadata_cur) == 0) {
            if (version != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_VERSION "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            version = eq + 1;
        } else if (strncmp(metadata_cur, "license", eq - metadata_cur) == 0) {
            if (license != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_LICENSE "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            license = eq + 1;
        } else if (strncmp(metadata_cur, "bslug", eq - metadata_cur) == 0) {
            if (bslug != NULL) {
                printf(
                    "Warning: Ignoring '%s' - Multiple BSLUG_MODULE_NAME "
                    "declarations.\n", path);
                module_has_info = true;
                goto exit_error;
            }
            bslug = eq + 1;
        }
    }
    
    if (game == NULL)
        game = "";
    if (bslug == NULL || strcmp(bslug, "0.1") != 0) {
        printf("Warning: Ignoring '%s' - Unrecognised BSlug version.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (name == NULL) {
        printf(
            "Warning: Ignoring '%s' - Missing BSLUG_MODULE_NAME declaration.\n",
            path);
        module_has_info = true;
        goto exit_error;
    }
    if (author == NULL) {
        printf(
            "Warning: Ignoring '%s' - Missing BSLUG_MODULE_AUTHOR "
            "declaration.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (version == NULL) {
        printf(
            "Warning: Ignoring '%s' - Missing BSLUG_MODULE_VERSION "
            "declaration.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    if (license == NULL) {
        printf(
            "Warning: Ignoring '%s' - Missing BSLUG_MODULE_LICENSE "
            "declaration.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    ret = malloc(
        sizeof(module_metadata_t) + strlen(path) +
        strlen(game) + strlen(name) + strlen(author) +
        strlen(version) + strlen(license) + 6);
    if (ret == NULL) {
        printf("Warning: Ignoring '%s' - Couldn't parse BSlug metadata.\n", path);
        module_has_info = true;
        goto exit_error;
    }
    
    tmp = (char *)(ret + 1);
    strcpy(tmp, path);
    ret->path = tmp;
    tmp = tmp + strlen(path) + 1;
    strcpy(tmp, game);
    ret->game = tmp;
    tmp = tmp + strlen(game) + 1;
    strcpy(tmp, name);
    ret->name = tmp;
    tmp = tmp + strlen(name) + 1;
    strcpy(tmp, author);
    ret->author = tmp;
    tmp = tmp + strlen(author) + 1;
    strcpy(tmp, version);
    ret->version = tmp;
    tmp = tmp + strlen(version) + 1;
    strcpy(tmp, license);
    ret->license = tmp;
    ret->size = 0;
    ret->entries_count = entries_count;
    
exit_error:
    if (metadata != NULL)
        free(metadata);
        
    return ret;
}

static bool Module_ElfLoadSection(
        const Elf *elf, Elf_Scn *scn, const Elf32_Shdr *shdr,
        void *destination) {
        
    assert(destination != NULL);
    
    switch (shdr->sh_type) {
        case SHT_SYMTAB:
        case SHT_PROGBITS: {
            Elf_Data *data;
            size_t n;
        
            n = 0;
            for (data = elf_getdata(scn, NULL);
                 data != NULL;
                 data = elf_getdata(scn, data)) {
                memcpy((char *)destination + n, data->d_buf, data->d_size);
                n += data->d_size;
            }
            return true;
        } case SHT_NOBITS: {
            memset(destination, 0, shdr->sh_size);
            return true;
        } default:
            return false;
    }
}

static void Module_ElfLoadSymbols(
        size_t shndx, const const void *destination,
        Elf32_Sym *symtab, size_t symtab_count) {
    
    size_t i;
    
    /* use the st_other field (no defined meaning) to indicate whether or not a
     * symbol address has been calculated. */
    for (i = 0; i < symtab_count; i++) {
        if (symtab[i].st_shndx == shndx &&
            symtab[i].st_other == 0) {
            
            symtab[i].st_value += (Elf32_Addr)destination;
            symtab[i].st_other = 1;
        }
    }
}

static bool Module_ElfLink(
        size_t index, Elf *elf, size_t shndx, void *destination,
        Elf32_Sym *symtab, size_t symtab_count, size_t symtab_strndx,
        bool allow_globals) {
    Elf_Scn *scn;
    
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
         
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
        
        switch (shdr->sh_type) {
            case SHT_REL: {
                const Elf32_Rel *rel;
                Elf_Data *data;
                size_t i;
                
                if (shdr->sh_info != shndx)
                    continue;
                
                data = elf_getdata(scn, NULL);
                if (data == NULL)
                    continue;
                    
                rel = data->d_buf;
                
                for (i = 0; i < shdr->sh_size / sizeof(Elf32_Rel); i++) {
                    uint32_t symbol_addr;
                    size_t symbol;
                    
                    symbol = ELF32_R_SYM(rel[i].r_info);
                    
                    if (symbol > symtab_count)
                        return false;
                    
                    switch (symtab[symbol].st_shndx) {
                        case SHN_ABS: {
                            symbol_addr = symtab[symbol].st_value;
                            break;
                        } case SHN_COMMON: {
                            return false;
                        } case SHN_UNDEF: {
                            if (allow_globals) {
                                module_unresolved_relocation_t *reloc;
                                char *name;
                                
                                reloc = Module_ListAllocate(
                                    &module_relocations,
                                    sizeof(module_unresolved_relocation_t), 1,
                                    &module_relocations_capacity,
                                    &module_relocations_count,
                                    MODULE_RELOCATIONS_CAPCITY_DEFAULT);
                                if (reloc == NULL)
                                    return false;
                                
                                name = elf_strptr(
                                    elf, symtab_strndx, symtab[symbol].st_name);
                                
                                if (name == NULL) {
                                    module_relocations_count--;
                                    return false;
                                }
                                
                                reloc->name = strdup(name);
                                if (reloc->name == NULL) {
                                    module_relocations_count--;
                                    return false;
                                }
                                
                                reloc->module = index;
                                reloc->address = destination;
                                reloc->offset = rel[i].r_offset;
                                reloc->type = ELF32_R_TYPE(rel[i].r_info);
                                reloc->addend = 
                                    *(int *)((char *)destination +
                                        rel[i].r_offset);
                                
                                continue;
                            } else
                                return false;
                        } default: {
                            if (symtab[symbol].st_other != 1)
                                return false;
                            
                            symbol_addr = symtab[symbol].st_value;
                            break;
                        }
                    }
                    
                    if (!Module_ElfLinkOne(
                            ELF32_R_TYPE(rel[i].r_info), rel[i].r_offset,
                            *(int *)((char *)destination + rel[i].r_offset),
                            destination, symbol_addr))
                        return false;
                }
                break;
            } case SHT_RELA: {
                const Elf32_Rela *rela;
                Elf_Data *data;
                size_t i;
                
                if (shdr->sh_info != shndx)
                    continue;
                
                data = elf_getdata(scn, NULL);
                if (data == NULL)
                    continue;
                    
                rela = data->d_buf;
                
                for (i = 0; i < shdr->sh_size / sizeof(Elf32_Rela); i++) {
                    uint32_t symbol_addr;
                    size_t symbol;
                    
                    symbol = ELF32_R_SYM(rela[i].r_info);
                    
                    if (symbol > symtab_count)
                        return false;
                    
                    switch (symtab[symbol].st_shndx) {
                        case SHN_ABS: {
                            symbol_addr = symtab[symbol].st_value;
                            break;
                        } case SHN_COMMON: {
                            return false;
                        } case SHN_UNDEF: {
                            if (allow_globals) {
                                module_unresolved_relocation_t *reloc;
                                char *name;
                                
                                reloc = Module_ListAllocate(
                                    &module_relocations,
                                    sizeof(module_unresolved_relocation_t), 1,
                                    &module_relocations_capacity,
                                    &module_relocations_count,
                                    MODULE_RELOCATIONS_CAPCITY_DEFAULT);
                                if (reloc == NULL)
                                    return false;
                                
                                name = elf_strptr(
                                    elf, symtab_strndx, symtab[symbol].st_name);
                                
                                if (name == NULL) {
                                    module_relocations_count--;
                                    return false;
                                }
                                
                                reloc->name = strdup(name);
                                if (reloc->name == NULL) {
                                    module_relocations_count--;
                                    return false;
                                }
                                
                                reloc->module = index;
                                reloc->address = destination;
                                reloc->offset = rela[i].r_offset;
                                reloc->type = ELF32_R_TYPE(rela[i].r_info);
                                reloc->addend = rela[i].r_addend;
                                
                                continue;
                            } else
                                return false;
                        } default: {
                            if (symtab[symbol].st_other != 1)
                                return false;
                            
                            symbol_addr = symtab[symbol].st_value;
                            break;
                        }
                    }
                                
                    if (!Module_ElfLinkOne(
                            ELF32_R_TYPE(rela[i].r_info), rela[i].r_offset,
                            rela[i].r_addend, destination, symbol_addr))
                        return false;
                }
                break;
            }
        }
    }
    
    return true;
}

static bool Module_ElfLinkOne(
        char type, size_t offset, int addend,
        void *destination, uint32_t symbol_addr) {
    int value;
    char *target = (char *)destination + offset;
    bool result = false;

    switch (type) {
        case R_PPC_ADDR32:
        case R_PPC_ADDR24:
        case R_PPC_ADDR16:
        case R_PPC_ADDR16_HI:
        case R_PPC_ADDR16_HA:
        case R_PPC_ADDR16_LO:
        case R_PPC_ADDR14:
        case R_PPC_ADDR14_BRTAKEN:
        case R_PPC_ADDR14_BRNTAKEN:
        case R_PPC_UADDR32:
        case R_PPC_UADDR16: {
            value = (int)symbol_addr + addend;
            break;
        } case R_PPC_REL24:
        case R_PPC_REL14:
        case R_PPC_REL14_BRTAKEN:
        case R_PPC_REL14_BRNTAKEN:
        case R_PPC_REL32:
        case R_PPC_ADDR30: {
            value = (int)symbol_addr + addend - (int)target;
            break;
        } case R_PPC_SECTOFF:
        case R_PPC_SECTOFF_LO:
        case R_PPC_SECTOFF_HI:
        case R_PPC_SECTOFF_HA: {
            value = offset + addend;
            break;
        } case R_PPC_EMB_NADDR32:
        case R_PPC_EMB_NADDR16:
        case R_PPC_EMB_NADDR16_LO:
        case R_PPC_EMB_NADDR16_HI:
        case R_PPC_EMB_NADDR16_HA: {
            value = addend - (int)symbol_addr;
            break;
        } default:
            goto exit_error;
    }
    
    
    switch (type) {
        case R_PPC_ADDR32:
        case R_PPC_UADDR32:
        case R_PPC_REL32:
        case R_PPC_SECTOFF:
        case R_PPC_EMB_NADDR32: {
            *(int *)target = value;
            break;
        } case R_PPC_ADDR24:
        case R_PPC_REL24: {
            *(int *)target =
                (*(int *)target & 0xfc000003) | (value & 0x03fffffc);
            break;
        } case R_PPC_ADDR16:
        case R_PPC_UADDR16:
        case R_PPC_EMB_NADDR16: {
            *(short *)target = value;
            break;
        } case R_PPC_ADDR16_HI:
        case R_PPC_SECTOFF_HI:
        case R_PPC_EMB_NADDR16_HI: {
            *(short *)target = value >> 16;
            break;
        } case R_PPC_ADDR16_HA:
        case R_PPC_SECTOFF_HA:
        case R_PPC_EMB_NADDR16_HA: {
            *(short *)target = (value >> 16) + ((value >> 15) & 1);
            break;
        } case R_PPC_ADDR16_LO:
        case R_PPC_SECTOFF_LO:
        case R_PPC_EMB_NADDR16_LO: {
            *(short *)target = value & 0xffff;
            break;
        } case R_PPC_ADDR14:
        case R_PPC_REL14: {
            *(int *)target =
                (*(int *)target & 0xffff0003) | (value & 0x0000fffc);
            break;
        } case R_PPC_ADDR14_BRTAKEN:
        case R_PPC_REL14_BRTAKEN: {
            *(int *)target =
                (*(int *)target & 0xffdf0003) | (value & 0x0000fffc) |
                0x00200000;
            break;
        } case R_PPC_ADDR14_BRNTAKEN:
        case R_PPC_REL14_BRNTAKEN: {
            *(int *)target =
                (*(int *)target & 0xffdf0003) | (value & 0x0000fffc);
            break;
        } case R_PPC_ADDR30: {
            *(int *)target =
                (*(int *)target & 0x00000003) | (value & 0xfffffffc);
            break;
        } default:
            goto exit_error;
    }
    
    result = true;
exit_error:
    if (!result) printf("Module_ElfLinkOne: exit_error\n");
    return result;
}

static bool Module_ListLink(uint8_t **space) {
    size_t i;
    bool result = false;
    
    for (i = 0; i < module_list_count; i++) {
        if (!Module_LinkModule(i, module_list[i]->path, space))
            goto exit_error;
    }
    
    result = true;
exit_error:
    if (!result) printf("Module_ListLink: exit_error\n");
    return result;
}

static bool Module_LinkModule(size_t index, const char *path, uint8_t **space) {
    int fd = -1;
    Elf *elf = NULL;
    bool result = false;
    
    /* check for compile errors */
    assert(elf_version(EV_CURRENT) != EV_NONE);
    
    fd = open(path, O_RDONLY, 0);
    
    if (fd == -1)
        goto exit_error;
        
    elf = elf_begin(fd, ELF_C_READ, NULL);
    
    if (elf == NULL)
        goto exit_error;
        
    switch (elf_kind(elf)) {
        case ELF_K_AR:
            /* TODO */
            goto exit_error;
        case ELF_K_ELF:
            if (!Module_LinkModuleElf(index, elf, space))
                goto exit_error;
            break;
        default:
            goto exit_error;
    }

    result = true;
exit_error:
    if (!result) printf("Module_LinkModule: exit_error\n");
    if (elf != NULL)
        elf_end(elf);
    if (fd != -1)
        close(fd);
    return result;
}

static bool Module_LinkModuleElf(size_t index, Elf *elf, uint8_t **space) {
    Elf_Scn *scn;
    size_t symtab_count, section_count, shstrndx, symtab_strndx, entries_count;
    Elf32_Sym *symtab = NULL;
    uint8_t **destinations = NULL;
    bslug_loader_entry_t *entries = NULL;
    bool result = false;
    
    if (!Module_LoadElfSymtab(elf, &symtab, &symtab_count, &symtab_strndx))
        goto exit_error;
    
    assert(symtab != NULL);
    
    if (elf_getshdrnum(elf, &section_count) != 0)
        goto exit_error;
    if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        goto exit_error;
    
    destinations = malloc(sizeof(uint8_t *) * section_count);
    
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
        
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
        
        if ((shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_NOBITS) && 
            (shdr->sh_flags & SHF_ALLOC)) {
            
            const char *name;
            
            destinations[elf_ndxscn(scn)] = NULL;
            
            name = elf_strptr(elf, shstrndx, shdr->sh_name);
            if (name == NULL)
                continue;
            
            if (strcmp(name, ".bslug.meta") == 0) {
                continue;
            } else if (strcmp(name, ".bslug.load") == 0) {
                if (entries != NULL)
                    goto exit_error;
                    
                entries_count = shdr->sh_size / sizeof(bslug_loader_entry_t);
                entries = Module_ListAllocate(
                    &module_entries, sizeof(bslug_loader_entry_t),
                    entries_count, &module_entries_capacity,
                    &module_entries_count, MODULE_ENTRIES_CAPACITY_DEFAULT);
                    
                if (entries == NULL)
                    goto exit_error;
                
                destinations[elf_ndxscn(scn)] = (uint8_t *)entries;
                if (!Module_ElfLoadSection(elf, scn, shdr, entries))
                    goto exit_error;
                Module_ElfLoadSymbols(
                    elf_ndxscn(scn), entries, symtab, symtab_count);
            } else {
                *space -= shdr->sh_size;
                if (shdr->sh_addralign > 3)
                    *space = (uint8_t *)((int)*space &
                        ~(shdr->sh_addralign - 1));
                else 
                    *space = (uint8_t *)((int)*space & ~3);
                
                destinations[elf_ndxscn(scn)] = *space;
                
                assert(*space != NULL);
                if (!Module_ElfLoadSection(elf, scn, shdr, *space))
                    goto exit_error;
                Module_ElfLoadSymbols(
                    elf_ndxscn(scn), *space, symtab, symtab_count);
            }
        }
    }
    
    if (entries == NULL)
        goto exit_error;
    
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
        
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr == NULL)
            continue;
        
        if ((shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_NOBITS) && 
            (shdr->sh_flags & SHF_ALLOC) &&
            destinations[elf_ndxscn(scn)] != NULL) {
            
            if (!Module_ElfLink(
                    index, elf, elf_ndxscn(scn), destinations[elf_ndxscn(scn)],
                    symtab, symtab_count, symtab_strndx, true))
                goto exit_error;
        }
    }
        
    result = true;
exit_error:
    if (!result) printf("Module_LinkModuleElf: exit_error\n");
    if (destinations != NULL)
        free(destinations);
    if (symtab != NULL)
        free(symtab);
    return result;
}

static bool Module_ListLoadSymbols(uint8_t **space) {
    size_t i;
    bool result = false;
    
    for (i = 0; i < module_entries_count; i++) {
        bslug_loader_entry_t *entry;

        entry = module_entries + i;
        
        switch (entry->type) {
        case BSLUG_LOADER_ENTRY_EXPORT: {
            if (!Search_SymbolAdd(
                entry->data.export.name,
                (void *)entry->data.export.target)) {
                
                printf("Could not export '%s'\n", entry->data.export.name);
                goto exit_error;
            }
                
            break;
        } case BSLUG_LOADER_ENTRY_FUNCTION:
        case BSLUG_LOADER_ENTRY_FUNCTION_MANDATORY: {
            break;
        } default:
            goto exit_error;
        }
    }
    
    result = true;
exit_error:
    if (!result) printf("Module_ListLoadSymbols: exit_error\n");
    return result;
}

static bool Module_ListLinkFinal(uint8_t **space) {
    size_t relocation_index, entry_index, module_index;
    bool result = false, has_error = false;
    
    relocation_index = 0;
    entry_index = 0;
    
    /* Process the replacements the link each module in turn.
     * It must be done in this order, otherwise two replacements to the same
     * function will cause an infinite loop. */
    for (module_index = 0; module_index < module_list_count; module_index++) {
        size_t limit;
        
        limit = entry_index + module_list[module_index]->entries_count;
        
        for (; entry_index < limit; entry_index++) {
            bslug_loader_entry_t *entry;
            
            assert(entry_index < module_entries_count);

            entry = module_entries + entry_index;
                
            switch (entry->type) {
            case BSLUG_LOADER_ENTRY_EXPORT: {
                break;
            } case BSLUG_LOADER_ENTRY_FUNCTION:
            case BSLUG_LOADER_ENTRY_FUNCTION_MANDATORY: {
                if (!Module_ListLinkFinalReplaceFunction(space, entry))
                    goto exit_error;
                break;
            } default:
                goto exit_error;
            }
        }
    
        for (; 
             relocation_index < module_relocations_count;
             relocation_index++) {
            module_unresolved_relocation_t *reloc;
            void *symbol;
            
            reloc = module_relocations + relocation_index;
            
            if (reloc->module != module_index)
                break;
            
            assert(reloc->name != NULL);
            symbol = Search_SymbolLookup(reloc->name);
            
            if (symbol == NULL) {
                printf(
                    "Missing symbol '%s' needed by '%s'\n", reloc->name,
                    module_list[module_index]->name);
                has_error = true;
                continue;
            }
            
            if (!Module_ElfLinkOne(
                    reloc->type, reloc->offset, reloc->addend,
                    reloc->address, (uint32_t)symbol))
                goto exit_error;
        }
    }
    
    assert(entry_index == module_entries_count);
    assert(relocation_index == module_relocations_count);
    
    if (has_error)
        goto exit_error;
    
    result = true;
exit_error:
    if (!result) printf("Module_ListLinkFinal: exit_error\n");
    module_relocations_count = 0;
    module_relocations_capacity = 0;
    free(module_relocations);
    return result;
}

static bool Module_ListLinkFinalReplaceFunction(
        uint8_t **space, bslug_loader_entry_t *entry) {
    bool result = false;
    uint32_t *data;
    
    assert(entry->type == BSLUG_LOADER_ENTRY_FUNCTION ||
           entry->type == BSLUG_LOADER_ENTRY_FUNCTION_MANDATORY);
    assert(entry->data.function.name != NULL);
    data = Search_SymbolLookup(entry->data.function.name);
    
    if (data == NULL) {
        if (entry->type == BSLUG_LOADER_ENTRY_FUNCTION) {
            *space -= 4;
            /* FIXME: this behaviour is bad; we should call abort or
             * some such.
             *
             * The hack is, if we're replacing a function, we probably
             * reference it. Since the symbol is missing, this won't
             * work, so we make up a plausible symbol table entry for
             * the purpose of relocation.
             */
            ((uint32_t *)*space)[0] = 0x48000000; /* spin loop */
            Search_SymbolAdd(entry->data.function.name, *space);
            goto exit_success;
        } else {
            printf("Missing symbol '%s'\n", entry->data.function.name);
            goto exit_error;
        }
    }
    
    switch (*data & 0xfc000002) {
        case 0x40000000: { /* bc */
            void *target;
            int offset;
                                
            offset = *data & 0x0000fffc;
            offset = (offset << 16) >> 16;
            target = (char *)data + offset;
        
            *space -= 12;
            /* Conditional branch to either target or next instruction.
             * We can't just conditional branch to original target as
             * it is probably too far for a single branch. */
            ((uint32_t *)*space)[0] = (*data & 0xffff0003) | 8;
            /* branch to second instruction of function.
             * observe: we're branching from the second instruction of 
             *     the stub to the second instruction of the method, so
             *     no offset is needed. */
            ((uint32_t *)*space)[1] =
                0x48000000 + 
                (((uint32_t)data - (uint32_t)*space) & 0x3fffffc);
            /* branch to the target of the original branch. */
            ((uint32_t *)*space)[2] =
                0x48000000 + 
                (((uint32_t)target - (uint32_t)(*space + 8))
                    & 0x3fffffc);
            break;
        } case 0x48000000: { /* b */
            void *target;
            int offset;
            
            offset = *data & 0x03fffffc;
            offset = (offset << 6) >> 6;
            target = (char *)data + offset;
            *space -= 8;
            /* branch to the target of the original branch. */
            ((uint32_t *)*space)[0] = 
                (*data & 0xfc000003) + 
                (((uint32_t)target - (uint32_t)*space) & 0x3fffffc);
            /* branch to second instruction of function.
             * observe: we're branching from the second instruction of 
             *     the stub to the second instruction of the method, so
             *     no offset is needed. */
            ((uint32_t *)*space)[1] =
                0x48000000 + 
                (((uint32_t)data - (uint32_t)*space) & 0x3fffffc);
            break;
        } default: { /* other (inc ba, and bca) */
            *space -= 8;
            /* copy the original instruction. */
            ((uint32_t *)*space)[0] = *data;
            /* branch to second instruction of function.
             * observe: we're branching from the second instruction of 
             *     the stub to the second instruction of the method, so
             *     no offset is needed. */
            ((uint32_t *)*space)[1] =
                0x48000000 + 
                (((uint32_t)data - (uint32_t)*space) & 0x3fffffc);
            break;
        }
    }
    
    *data = 0x48000000 + 
        (((uint32_t)entry->data.function.target
            - (uint32_t)data) & 0x3fffffc);
    DCFlushRange((void *)((uint32_t)data & ~31), 32);
    ICInvalidateRange((void *)((uint32_t)data & ~31), 32);
    
    if (!Search_SymbolReplace(entry->data.function.name, *space))
        goto exit_error;

exit_success:
    result = true;
exit_error:
    if (!result) printf("Module_ListLinkFinalReplaceFunction: exit_error\n");
    return result;
}
