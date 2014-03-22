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
#include <bslug/bslug.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libelf.h>
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
#include "threads.h"

typedef struct {
    const char *name;
    void *address;
    char type;
    int addend;
} module_unresolved_relocation_t;

typedef struct {
    const char *name;
    void *address;
} module_symbol_t;

event_t module_fat_loaded;
event_t module_list_loaded;

#define MODULE_LIST_CAPACITY_DEFAULT 16

size_t module_list_size = 0;
module_metadata_t **module_list = NULL;
size_t module_list_count = 0;
static size_t module_list_capacity = 0;

#define MODULE_RELOCATIONS_CAPCITY_DEFAULT 128

static module_unresolved_relocation_t *module_relocations = NULL;
static size_t module_relocations_count = 0;
static size_t module_relocations_capacity = 0;

#define MODULE_SYMBOLS_DEFAULT 128

static module_symbol_t **module_symbols = NULL;
static size_t module_symbols_count = 0;
static size_t module_symbols_capacity = 0;

static const char module_path[] = "sd:/bslug/modules";

static void *Module_Main(void *arg);
static void Module_ListLoad(void);
static void Module_CheckDirectory(char *path);
static void Module_CheckFile(const char *path);
static void Module_Load(const char *path);
static void Module_LoadElf(const char *path, Elf *elf);
static bool Module_LoadElfSymtab(
    Elf *elf, size_t *symtab_strndx, Elf32_Sym **symtab, size_t *symtab_count);
static module_metadata_t *Module_MetadataRead(
    const char *path, Elf *elf, Elf32_Sym *symtab, size_t symtab_count);
static bool Module_ElfLoadSection(
    const Elf *elf, Elf_Scn *scn, const Elf32_Shdr *shdr, void *destination);
static void Module_ElfLoadSymbols(
    size_t shndx, const const void *destination, 
    Elf32_Sym *symtab, size_t symtab_count);
static bool Module_ElfLink(
    Elf *elf, size_t shndx, void *destination, size_t offset, size_t size,
    Elf32_Sym *symtab, size_t symtab_count, bool allow_globals);
static bool Module_ElfLinkOne(
    Elf *elf, char type, size_t offset, size_t symbol, int addend,
    void *destination, Elf32_Sym *symtab, size_t symtab_count,
    bool allow_globals);
static bool Module_ListLink(void);

bool Module_Init(void) {
    return Event_Init(&module_list_loaded);
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
    Module_ListLoad();
    
    Event_Trigger(&module_list_loaded);
    
    return NULL;
}

static void *Module_ListAllocate(
        void **list, size_t entry_size, size_t *capacity, size_t *count,
        size_t default_capacity) {
    void *result;
    
    if (*capacity == *count) {
        if (*count == 0) {
            *list = malloc(entry_size * default_capacity);
            
            if (*list == NULL)
                return NULL;
            
            *capacity = default_capacity
        } else {
            void *temp;
            
            temp = realloc(*list, entry_size * *capacity * 2);
            if (temp == NULL)
                return NULL;
            
            *list = temp;
            *capacity = *capacity * 2;
        }
    }
    
    result = (char *)*list + entry_size * *count;
    (*count)++;
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
            Module_LoadElf(path, elf);
            break;
        default:
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
    
    if (ident == NULL)
        goto exit_error;
    if (sz < 7)
        goto exit_error;
    if (ident[4] != ELFCLASS32)
        goto exit_error;
    if (ident[5] != ELFDATA2MSB)
        goto exit_error;
    if (ident[6] != EV_CURRENT)
        goto exit_error;
        
    ehdr = elf32_getehdr(elf);
    
    if (ehdr == NULL)
        goto exit_error;
    if (ehdr->e_type != ET_REL)
        goto exit_error;
    if (ehdr->e_machine != EM_PPC)
        goto exit_error;
    if (ehdr->e_version != EV_CURRENT)
        goto exit_error;
        
    if (!Module_LoadElfSymtab(elf, &symtab_strndx, &symtab, &symtab_count))
        goto exit_error;
        
    assert(symtab != NULL);
        
    metadata = Module_MetadataRead(path, elf, symtab, symtab_count);
    
    if (metadata == NULL)
        goto exit_error;
    
    if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        goto exit_error;
    
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
            } else if (strcmp(name, ".bslug.data") == 0) {
                continue;
            } else if (strcmp(name, ".bslug.load") == 0) {
                metadata->size += shdr->sh_size / sizeof(bslug_loader_entry_t);
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
        module_list, sizeof(module_metadata_t *), &module_list_capacity,
        &module_list_count, MODULE_LIST_CAPACITY_DEFAULT);
    if (list_ptr == NULL)
        goto exit_error;
    
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
        Elf *elf, size_t *symtab_strndx, Elf32_Sym **symtab,
        size_t *symtab_count) {
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
        const char *path, Elf *elf, Elf32_Sym *symtab, size_t symtab_count) {
    char *metadata = NULL, *metadata_cur, *metadata_end, *tmp;
    const char *game, *name, *author, *version, *license;
    module_metadata_t *ret = NULL;
    Elf_Scn *scn;
    size_t shstrndx;
    
    if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        goto exit_error;
    
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
            
            assert(metadata == NULL);
            metadata = malloc(shdr->sh_size);
            if (metadata == NULL)
                continue;
                
            if (!Module_ElfLoadSection(elf, scn, shdr, metadata))
                goto exit_error;
            
            Module_ElfLoadSymbols(
                elf_ndxscn(scn), metadata, symtab, symtab_count);
            
            if (!Module_ElfLink(
                    elf, elf_ndxscn(scn), metadata, 0, shdr->sh_size,
                    symtab_strndx, symtab, symtab_count, false))
                goto exit_error;
            
            metadata_end = metadata + shdr->sh_size;
            metadata_end[-1] = '\0';
            break;
        }
    }

    if (metadata == NULL)
        goto exit_error;
    
    game = NULL;
    name = NULL;
    author = NULL;
    version = NULL;
    license = NULL;
    
    for (metadata_cur = metadata;
         metadata_cur < metadata_end;
         metadata_cur = strchr(metadata_cur, '\0') + 1) {
         
        char *eq;
         
        assert(metadata_cur >= metadata && metadata_cur < metadata_end);
        
        if (*metadata_cur == '\0')
            continue;
        
        eq = strchr(metadata_cur, '=');
        if (eq == NULL)
            goto exit_error;
        
        if (strncmp(metadata_cur, "game", eq - metadata_cur) == 0) {
            if (game != NULL)
                goto exit_error;
            game = eq + 1;
        } else if (strncmp(metadata_cur, "name", eq - metadata_cur) == 0) {
            if (name != NULL)
                goto exit_error;
            name = eq + 1;
        } else if (strncmp(metadata_cur, "author", eq - metadata_cur) == 0) {
            if (author != NULL)
                goto exit_error;
            author = eq + 1;
        } else if (strncmp(metadata_cur, "version", eq - metadata_cur) == 0) {
            if (version != NULL)
                goto exit_error;
            version = eq + 1;
        } else if (strncmp(metadata_cur, "license", eq - metadata_cur) == 0) {
            if (license != NULL)
                goto exit_error;
            license = eq + 1;
        } else
            goto exit_error;
    }
    
    if (game == NULL)
        game = "";
    if (name == NULL || author == NULL || version == NULL || license == NULL)
        goto exit_error;
    
    ret = malloc(
        sizeof(module_metadata_t) + strlen(path) +
        strlen(game) + strlen(name) + strlen(author) +
        strlen(version) + strlen(license) + 6);
    if (ret == NULL)
        goto exit_error;
    
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
    
exit_error:
    if (metadata != NULL)
        free(metadata);
        
    return ret;
}

static bool Module_ElfLoadSection(
        const Elf *elf, Elf_Scn *scn, const Elf32_Shdr *shdr,
        void *destination) {
    
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
        Elf *elf, size_t shndx, void *destination, size_t offset, size_t size,
        Elf32_Sym *symtab, size_t symtab_strndx, size_t symtab_count,
        bool allow_globals) {
    Elf_Scn *scn;
    char *start;
    
    start = destination - size;
        
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
         
        Elf32_Shdr *shdr;
        
        shdr = elf32_getshdr(scn);
        if (shdr != NULL)
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
                    /* check this relocation is inside the region */
                    if (rel[i].r_offset + 4 <= offset ||
                        rel[i].r_offset > offset + size) 
                        continue;
                    
                    /* reject relocations staddling the region */
                    if (rel[i].r_offset < offset ||
                        rel[i].r_offset + 4 > offset + size)
                        return false;
                    
                    if (!Module_ElfLinkOne(
                            elf, ELF32_R_TYPE(rel[i].r_info), rel[i].r_offset,
                            ELF32_R_SYM(rel[i].r_info), 
                            *(int *)(start + rel[i].r_offset),
                            start, symtab_strndx, symtab, symtab_count,
                            allow_globals))
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
                    /* check this relocation is inside the region */
                    if (rela[i].r_offset + 4 <= offset ||
                        rela[i].r_offset > offset + size) 
                        continue;
                    
                    /* reject relocations staddling the region */
                    if (rela[i].r_offset < offset ||
                        rela[i].r_offset + 4 > offset + size)
                        return false;
                    
                    if (!Module_ElfLinkOne(
                            elf, ELF32_R_TYPE(rela[i].r_info), rela[i].r_offset, 
                            ELF32_R_SYM(rela[i].r_info), rela[i].r_addend,
                            start, symtab_strndx, symtab, symtab_count,
                            allow_globals))
                        return false;
                }
                break;
            }
        }
    }
    
    return true;
}

static bool Module_ElfLinkOne(
        Elf *elf, char type, size_t offset, size_t symbol, int addend,
        void *destination, size_t symtab_strndx, Elf32_Sym *symtab,
        size_t symtab_count, bool allow_globals) {
    int value;
    char *target = (char *)destination + offset;
    Elf32_Addr symbol_addr;
    
    switch (type) {
        case R_PPC_ADDR32:
        case R_PPC_ADDR16:
        case R_PPC_UADDR32:
        case R_PPC_UADDR16: {
            if (symbol > symtab_count)
                return false;
            
            switch (symtab[symbol].st_shndx) {
                case SHN_ABS: {
                    symbol_addr = symtab[symbol].st_value;
                    break;
                } case SHN_COMMON:
                    return false;
                case SHN_UNDEF: {
                    if (allow_globals) {
                        module_unresolved_relocation_t *reloc;
                        char *name;
                        
                        reloc = Module_ListAllocate(
                            module_relocations,
                            sizeof(module_unresolved_relocation_t),
                            &module_relocations_capacity,
                            &module_relocations_count,
                            MODULE_RELOCATIONS_CAPCITY_DEFAULT);
                        if (reloc == NULL)
                            goto exit_error;
                        
                        name = elf_strptr(
                            elf, symtab_strndx, symtab[symbol].st_name);
                        
                        reloc->name = strdup(name);
                        if (reloc->name == NULL) {
                            module_relocations_count--;
                            goto exit_error;
                        }
                        
                        reloc->address = target;
                        reloc->type = type;
                        reloc->addend = addend;
                        
                        return true;
                    } else
                        return false;
                } default: {
                    if (symtab[symbol].st_other != 1)
                        return false;
                    
                    symbol_addr = symtab[symbol].st_value;
                    break;
                }
            }            
            break;
        } default:
            return false;
    }

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
            value = symbol_addr + addend;
        } case R_PPC_REL24:
        case R_PPC_REL14:
        case R_PPC_REL14_BRTAKEN:
        case R_PPC_REL14_BRNTAKEN:
        case R_PPC_REL32:
        case R_PPC_ADDR30: {
            value = symbol_addr + addend - (int)target;
        } case R_PPC_SECTOFF:
        case R_PPC_SECTOFF_LO:
        case R_PPC_SECTOFF_HI:
        case R_PPC_SECTOFF_HA: {
            value = offset + addend;
        } case R_PPC_EMB_NADDR32:
        case R_PPC_EMB_NADDR16:
        case R_PPC_EMB_NADDR16_LO:
        case R_PPC_EMB_NADDR16_HI:
        case R_PPC_EMB_NADDR16_HA: {
            value = addend - symbol_addr;
        } default:
            return false;
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
            return false;
    }
}

static bool Module_ListLink(void) {
    size_t i;
    uint8_t *space;
    Elf *elf = NULL;
    bool result = false;
    
    space = (uint8_t *)0x81800000;
    
    for (i = 0; i < module_list_count; i++) {
        if (!Module_LinkModule(module_list[i]->path), &space)
            goto exit_error;
    }
    
    assert(space + module_list_size <= (uint8_t *)0x81800000);
    
    result = true;
exit_error:
    return result;
}

static bool Module_LinkModule(const void char *path, uint8_t **space) {
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
            if (!Module_LinkModuleElf(elf, space))
                goto exit_error;
            break;
        default:
            goto exit_error;
    }

    result = true;
exit_error:
    if (elf != NULL)
        elf_end(elf);
    if (fd != -1)
        close(fd);
    return result;
}

static bool Module_LinkModuleElf(Elf *elf, uint8_t **space) {
    Elf_Scn *scn;
    size_t symtab_count, section_count, shstrndx, symtab_strndx, entries_count;
    Elf32_Sym *symtab = NULL;
    uint8_t *data = NULL, **destinations = NULL;
    bslug_loader_entry_t *entries = NULL;
    bool result = false;
        
    if (!Module_LoadElfSymtab(elf, &symtab_strndx, &symtab, &symtab_count))
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
                
            name = elf_strptr(elf, shstrndx, shdr->sh_name);
            if (name == NULL)
                continue;
            
            if (strcmp(name, ".bslug.meta") == 0) {
                continue;
            } else if (strcmp(name, ".bslug.data") == 0) {
                if (data != NULL)
                    goto exit_error;
                data = malloc(shdr->sh_size);
                if (data == NULL)
                    goto exit_error;
                
                destinations[elf_ndxscn(scn)] = data;
                if (!Module_ElfLoadSection(elf, scn, shdr, data))
                    goto exit_error;
            } else if (strcmp(name, ".bslug.load") == 0) {
                if (entries != NULL)
                    goto exit_error;
                entries = malloc(shdr->sh_size);
                if (entries == NULL)
                    goto exit_error;
                
                destinations[elf_ndxscn(scn)] = entries;
                if (!Module_ElfLoadSection(elf, scn, shdr, entries))
                    goto exit_error;
                Module_ElfLoadSymbols(
                    elf_ndxscn(scn), entries, symtab, symtab_count);
                entries_count = shdr->sh_size / sizeof(bslug_loader_entry_t);
            } else {
                *space -= shdr->sh_size;
                if (shdr->sh_addralign > 3)
                    *space = (uint8_t *)(-(int)*space & 
                        (shdr->sh_addralign - 1));
                else 
                    *space = (uint8_t *)(-(int)*space & 3);
                
                destinations[elf_ndxscn(scn)] = space;
                if (!Module_ElfLoadSection(elf, scn, shdr, space))
                    goto exit_error;
                Module_ElfLoadSymbols(
                    elf_ndxscn(scn), space, symtab, symtab_count);
            }
        }
    }
    
    if (entries != NULL) {
        size_t i;
        
        for (i = 0; i < entries_count; i++) {
            switch (entries[i].type) {
                case BSLUG_LOADER_ENTRY_FUNCTION: {
                    
                    break;
                } case BSLUG_LOADER_ENTRY_LOAD: {
                    Event_Wait(&apploader_event_complete);
                                      
                    break;
                } case BSLUG_LOADER_ENTRY_GLOBAL: {
                    
                    break;
                } default:
                    goto exit_error;
            }
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
            
            if (!Module_ElfLink(
                    elf, elf_ndxscn(scn), destinations[elf_ndxscn(scn)],
                    0, symtab_strndx, shdr->sh_size, symtab, symtab_count))
                goto exit_error;
        }
    }
    
    result = true:
exit_error:
    if (destinations != NULL)
        free(destinations);
    if (entries != NULL)
        free(entries);
    if (data != NULL)
        free(data);
    if (symtab != NULL)
        free(symtab);
    return result;
}
