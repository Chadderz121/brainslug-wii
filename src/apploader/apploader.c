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
#include <ogc/lwp.h>
#include <stdio.h>

#include "di/di.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "threads.h"

event_t apploader_event_disk_id;

static void *aploader_main(void *arg);
static char apploader_ipc_tmd[18944] ATTRIBUTE_ALIGN(32);

int apploader_run_background(void) {
    int ret;
    lwp_t thread;
    
    Event_Init(&apploader_event_disk_id);
    
    ret = LWP_CreateThread(
        &thread, &aploader_main,
        NULL, NULL, 0, THREAD_PRIO_IO);
        
    if (ret) {
        errno = ENOMEM;
        return -1;
    }
    
    return 0;
}

typedef struct {
    uint32_t boot_info_count;
    uint32_t partition_info_offset;
} contents_t;

typedef struct {
    uint32_t offset;
    uint32_t length;
} partition_info_t;

// types for the four methods called on the game's apploader
typedef void (*app_init_t)(void (*report)(const char *fmt, ...));
typedef int (*app_main_t)(void **dst, int *size, int *offset);
typedef void (*game_entry_t)(void);
typedef game_entry_t (*app_final_t)(void);
typedef void (*app_entry_t)(
    app_init_t *init,
    app_main_t* main,
    app_final_t* final);
    
static void *aploader_main(void *arg) {
    int ret, i;
    contents_t ipc_toc[4] ATTRIBUTE_ALIGN(32);
    partition_info_t ipc_partition_info[4] ATTRIBUTE_ALIGN(32);
    uint32_t ipc_buffer[8] ATTRIBUTE_ALIGN(32);
    partition_info_t *boot_partition;
    tmd *dvd_tmd;
    app_init_t app_init;
    app_main_t app_main;
    app_final_t app_final;
    app_entry_t app_entry;
    game_entry_t game_entry;
    
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
    do {
        ret = DI_ReadUnencrypted(
            ipc_partition_info, sizeof(ipc_partition_info),
            ipc_toc->part_info_offset);
    } while (ret < 0);
    
    boot_partition = NULL;
    for (i = 0; i < ipc_toc->boot_info_count; i++) {
        if (ipc_partition_info[i].len == 0) {
            boot_partition = &ipc_partition_info[i];
        }
    }
    
    do {
        ret = DI_PartitionOpen(
            boot_partition->offset, (void *)apploader_ipc_tmd);
    } while (ret < 0);
    
    dvd_tmd = SIGNATURE_PAYLOAD((u32*)apploader_ipc_tmd);
    
    do {
        ret = DI_Read(ipc_buffer, sizeof(ipc_buffer), 0x2440 / 4);
    } while (ret < 0);
    DCFlushRange(ipc_buffer, sizeof(ipc_buffer));
    
    do {
        ret = DI_Read(
            (void*)0x81200000, (ipc_buffer[5] + 31) & ~31, 0x2460 / 4);
    } while (ret < 0);
    
    app_entry = (app_entry_t)ipc_buffer[4];
    
    app_entry(&app_init, &app_main, &app_final);
    
    app_init(&printf);
    
    settime(secs_to_ticks(time(NULL) - 946684800));
    
    while (1) {
        void* destination = 0;
        int length = 0, offset = 0;
        
        ret = app_main(&destination, &len, &offset);
        if (!ret)
            break;

        if (destination >= (void *)&parameters)
            destination = (char *)destination - PARAMETERS_SIZE;

        do {
            ret = DI_Read(destination, len, offset / 4 << 2);
        } while (ret < 0);
        
        DCFlushRange(destination, len);
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
    os0->info.arenahigh = (char *)os0->info.arenahigh - PARAMETERS_SIZE;
    os0->info.fst = (char *)os0->info.fst - PARAMETERS_SIZE;
    os0->info.fst_size += PARAMETERS_SIZE;

    os0->threads.debug_monitor_location = (void *)0x81800000;
    os0->threads.simulated_memory_size = 0x01800000;
    os0->threads.bus_speed = 0x0E7BE2C0;
    os0->threads.cpu_speed = 0x2B73A840;

    os1->fst = os0->info.fst;

    memcpy(os1->application_name, os0->disc.gamename, 4);

    DCFlushRange(os0, 0x3f00);
    
    game_entry = app_final();
    
    
    
    return NULL;
}