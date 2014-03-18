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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <ogc/lwp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "apploader/apploader.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "main.h"
#include "threads.h"

event_t module_fat_loaded;
event_t module_list_loaded;
size_t module_list_size = 0;

static const char module_path[] = "sd:/bslug/modules";

static void *Module_Main(void *arg);
static void Module_ListLoad(void);
static void Module_CheckDirectory(char *path);
static void Module_CheckFile(const char *path);
static void Module_Load(const char *path);
static void Module_LoadElf(Elf *elf);

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
            break;
        case ELF_K_ELF:
            Module_LoadElf(elf);
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

static void Module_LoadElf(Elf *elf) {
    Elf_Scn *scn;
    Elf_Data *data;
    size_t shstrndx;
    
    assert(elf != NULL);
    assert(elf_kind(elf) == ELF_K_ELF);
    
    if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        return;
        
    for (scn = elf_nextscn(elf, NULL);
         scn != NULL;
         scn = elf_nextscn(elf, scn)) {
         
        GElf_Shdr shdr;
         
        if (gelf_getshdr(scn, &shdr) != &shdr)
            continue;
    }
}
