/* search.c
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

/* This file should ideally avoid Wii specific methods so unit testing can be
 * conducted elsewhere. */
 
#include "search.h"

#include <dirent.h>
#include <errno.h>
#include <ogc/lwp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "apploader/apploader.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "search/fsm.h"
#include "search/symbol.h"
#include "main.h"
#include "threads.h"

event_t search_event_completed;

static fsm_t *search_fsm = NULL;

static const char search_path[] = "sd:/bslug/symbols";

static void *Search_Main(void *arg);
static void Search_SymbolsLoad(void);
static void Search_CheckDirectory(char *path);
static void Search_CheckFile(const char *path);
static void Search_Load(const char *path);
static bool Search_BuildFSM(void);
static void Search_SymbolMatch(symbol_index_t symbol, const uint8_t *addr);

bool Search_Init(void) {
    return Event_Init(&search_event_completed);
}

bool Search_RunBackground(void) {
    int ret;
    lwp_t thread;
    
    ret = LWP_CreateThread(
        &thread, &Search_Main,
        NULL, NULL, 0, THREAD_PRIO_IO);
        
    if (ret) {
        errno = ENOMEM;
        return false;
    }
    
    return true;
}

static void *Search_Main(void *arg) {
    Search_SymbolsLoad();
    
    if (symbol_count > 0) {
        if (!Search_BuildFSM())
           goto exit_error;
        
        assert(search_fsm != NULL);
        
        Event_Wait(&apploader_event_complete);
        
        if (apploader_app0_start != NULL) {
            assert(apploader_app0_end != NULL);
            assert(apploader_app0_end >= apploader_app0_start);
            
            FSM_Run(
                search_fsm, apploader_app0_start,
                apploader_app0_end - apploader_app0_start,
                &Search_SymbolMatch);
        }
    }
    
exit_error:
    Event_Trigger(&search_event_completed);
    return NULL;
}

static void Search_SymbolsLoad(void) {
    char path[FILENAME_MAX];

    Event_Wait(&main_event_fat_loaded);
    
    assert(sizeof(path) > sizeof(search_path));
    
    strcpy(path, search_path);
    
    Search_CheckDirectory(path);
}

static void Search_CheckDirectory(char *path) {
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
                    
                    Search_CheckFile(path);
                    
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
                        
                        Search_CheckDirectory(path);
                        
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

static void Search_CheckFile(const char *path) {
    const char *extension;
    
    /* find the file extension */
    extension = strrchr(path, '.');
    if (extension == NULL)
        extension = strchr(path, '\0');
    else
        extension++;
        
    assert(extension != NULL);
    
    if (strcmp(extension, "xml") == 0) {
        Search_Load(path);
    }
}

static void Search_Load(const char *path) {
    FILE *file = NULL;
    
    file = fopen(path, "r");
    if (file == NULL)
        goto exit_error;
    
    if (!Symbol_ParseFile(file))
        goto exit_error;
    
exit_error:
    if (file != NULL)
        fclose(file);
}

static bool Search_BuildFSM(void) {
    bool result = false;
    symbol_index_t i;
    fsm_t *fsm_final = NULL;
    
    for (i = 0; i < symbol_count; i++) {
        fsm_t *fsm;

        fsm = FSM_Create(i);
        if (fsm == NULL)
            goto exit_error;
        
        if (fsm_final == NULL) {
            fsm_final = fsm;
        } else {
            fsm_t *fsm_merge;
            
            fsm_merge = FSM_Merge(fsm_final, fsm);
            if (fsm_merge == NULL) {
                FSM_Free(fsm);
                goto exit_error;
            }
            
            FSM_Free(fsm);
            FSM_Free(fsm_final);
            
            fsm_final = fsm_merge;
        }
    }
    
    /* store the final result and make sure we don't free it!! */
    search_fsm = fsm_final;
    fsm_final = NULL;
    
    result = true;
exit_error:
    if (fsm_final != NULL)
        FSM_Free(fsm_final);
    return result;
}

static void Search_SymbolMatch(symbol_index_t symbol, const uint8_t *addr) {
    symbol_t *symbol_data;
    
    symbol_data = Symbol_GetSymbol(symbol);
    if (symbol_data == NULL)
        return;
    
    if (symbol_data->name != NULL)
        printf("Symbol %s found at %p\n", symbol_data->name, addr);
    else
        printf("Symbol %u found at %p\n", symbol, addr);
}