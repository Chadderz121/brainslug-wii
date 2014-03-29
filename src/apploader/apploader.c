/* apploader.c
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

#include "apploader.h"

#include <errno.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <ogc/lwp_watchdog.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "di/di.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "modules/module.h"
#include "threads.h"

typedef struct {
    uint32_t boot_info_count;
    uint32_t partition_info_offset;
} contents_t;

typedef struct {
    uint32_t offset;
    uint32_t type;
} partition_info_t;

// types for the four methods called on the game's apploader
typedef void (*apploader_report_t)(const char *format, ...);
typedef void (*apploader_init_t)(apploader_report_t report_fn);
typedef int (*apploader_main_t)(void **dst, int *size, int *offset);
typedef apploader_game_entry_t (*apploader_final_t)(void);
typedef void (*apploader_entry_t)(
    apploader_init_t *init,
    apploader_main_t *main,
    apploader_final_t *final);

event_t apploader_event_disk_id;
event_t apploader_event_complete;
apploader_game_entry_t apploader_game_entry_fn = NULL;
uint8_t *apploader_app0_start = NULL;
uint8_t *apploader_app0_end = NULL;
uint8_t *apploader_app1_start = NULL;
uint8_t *apploader_app1_end = NULL;

#define APPLOADER_APP0_BOUNDARY ((void *)0x81200000)
#define APPLOADER_APP1_BOUNDARY ((void *)0x81400000)

static u32 apploader_ipc_tmd[0x4A00 / 4] ATTRIBUTE_ALIGN(32);

static void *Aploader_Main(void *arg);

bool Apploader_Init(void) {
    return 
        Event_Init(&apploader_event_disk_id) &&
        Event_Init(&apploader_event_complete);
}

bool Apploader_RunBackground(void) {
    int ret;
    lwp_t thread;
    
    ret = LWP_CreateThread(
        &thread, &Aploader_Main,
        NULL, NULL, 0, THREAD_PRIO_IO);
        
    if (ret) {
        errno = ENOMEM;
        return false;
    }
    
    return true;
}

void Apploader_Report(const char *format, ...) {
#if 0
    /* debugging code, uncomment to display apploader logging messages */
    va_list args;

    va_start(args, format);
    vprintf(message, sizeof(message), format, args);
    va_end(args);
#endif
}
    
static void *Aploader_Main(void *arg) {
    int ret, i;
    contents_t ipc_toc[4] ATTRIBUTE_ALIGN(32);
    partition_info_t ipc_partition_info[4] ATTRIBUTE_ALIGN(32);
    uint32_t ipc_buffer[8] ATTRIBUTE_ALIGN(32);
    partition_info_t *boot_partition;
    apploader_init_t fn_init;
    apploader_main_t fn_main;
    apploader_final_t fn_final;
    apploader_entry_t fn_entry;
    
    do {
        ret = DI_Init();
    } while (ret);
    
    do {
        ret = DI_DiscInserted();
    } while (ret < 0);
    if (!ret) {
        do {
            ret = DI_DiscWait();
        } while (ret != 4);
    }
    
    do {
        ret = DI_Reset();
    } while (ret < 0);
    
    Event_Trigger(&apploader_event_disk_id);
    
    do {
        ret = DI_ReadUnencrypted(ipc_toc, sizeof(ipc_toc), 0x00010000);
    } while (ret < 0);
    DCInvalidateRange(ipc_toc, sizeof(ipc_toc));
    do {
        ret = DI_ReadUnencrypted(
            ipc_partition_info, sizeof(ipc_partition_info),
            ipc_toc->partition_info_offset);
    } while (ret < 0);
    
    boot_partition = NULL;
    for (i = 0; i < ipc_toc->boot_info_count; i++) {
        if (ipc_partition_info[i].type == 0) {
            boot_partition = &ipc_partition_info[i];
        }
    }
    
    do {
        ret = DI_PartitionOpen(
            boot_partition->offset, (void *)apploader_ipc_tmd);
    } while (ret < 0);
    
#if 0
    /* debugging code */
    {
        tmd *dvd_tmd;
        
        dvd_tmd = SIGNATURE_PAYLOAD(apploader_ipc_tmd);
        
        printf(
            "Title ID: %08x-%.4s\nIOS: %08x-IOS%d\n",
            (int)(dvd_tmd->title_id >> 32), (char *)&dvd_tmd->title_id + 4,
            (int)(dvd_tmd->sys_version >> 32), (int)dvd_tmd->sys_version);
    }
#endif
    
    do {
        ret = DI_Read(ipc_buffer, sizeof(ipc_buffer), 0x2440 / 4);
    } while (ret < 0);
    
    do {
        ret = DI_Read(
            (void*)0x81200000, (ipc_buffer[5] + 31) & ~31, 0x2460 / 4);
    } while (ret < 0);
    
    fn_entry = (apploader_entry_t)ipc_buffer[4];
    
    fn_entry(&fn_init, &fn_main, &fn_final);   
    fn_init(&Apploader_Report);
    
    settime(secs_to_ticks(time(NULL) - 946684800));

    Event_Wait(&module_event_list_loaded);
    
    while (1) {
        void* destination = 0;
        int length = 0, offset = 0;
        
        ret = fn_main(&destination, &length, &offset);
        if (!ret)
            break;
        
        if (destination < APPLOADER_APP0_BOUNDARY) {
            uint8_t *range_start, *range_end;

            range_start = destination;
            range_end = range_start + length;
            
            if (apploader_app0_start == NULL ||
                range_start < apploader_app0_start) {
                
                apploader_app0_start = range_start;
            }
            if (apploader_app0_end == NULL ||
                range_end > apploader_app0_end) {
                
                apploader_app0_end = range_end;
            }
        } else if (destination > APPLOADER_APP1_BOUNDARY) {
            uint8_t *range_start, *range_end;
            
            destination = (char *)destination - module_list_size;
            
            range_start = destination;
            range_end = range_start + length;
            
            if (apploader_app1_start == NULL ||
                range_start < apploader_app1_start) {
                
                apploader_app1_start = range_start;
            }
            if (apploader_app1_end == NULL ||
                range_end > apploader_app1_end) {
                
                apploader_app1_end = range_end;
            }
        }

        do {
            ret = DI_Read(destination, length, offset & ~3);
        } while (ret < 0);
        
        DCFlushRange(destination, length);
    }
        
    switch (os0->disc.gamename[3]) {
        case 'E':
        case 'J':
            os0->threads.tv_mode = OS_TV_MODE_NTSC;
            break;
        case 'P':
        case 'D':
        case 'F':
        case 'X':
        case 'Y':
            os0->threads.tv_mode = OS_TV_MODE_PAL;
            break;
    }
    
    os0->info.boot_type = OS_BOOT_NORMAL;
    os0->info.version = 1;
    os0->info.mem1_size = 0x01800000;
    os0->info.console_type = 1 + ((*(uint32_t *)0xcc00302c) >> 28);
    os0->info.arena_high = os0->info.arena_high - module_list_size;
    os0->info.fst = (char *)os0->info.fst - module_list_size;
    os0->info.fst_size += module_list_size;

    os0->threads.debug_monitor_location = (void *)0x81800000;
    os0->threads.simulated_memory_size = 0x01800000;
    os0->threads.bus_speed = 0x0E7BE2C0;
    os0->threads.cpu_speed = 0x2B73A840;
    
    /* FIXME: We don't currently reload IOS. To prevent Error #002 we pretend
     * like we have. */
    os1->ios_number = os1->expected_ios_number;
    os1->ios_revision = os1->expected_ios_revision;

    os1->fst = os0->info.fst;
    memcpy(os1->application_name, os0->disc.gamename, 4);

    DCFlushRange(os0, 0x3f00);
    
    apploader_game_entry_fn = fn_final();

    Event_Trigger(&apploader_event_complete);
    
    return NULL;
}
