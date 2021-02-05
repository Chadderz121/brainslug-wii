/* apploader_get_ios.c
 *   by Florian Bach, based on apploader.c by Alex Chadwick
 * 
 * Copyright (C) 2014, Alex Chadwick
 * Copyright (C) 2020, Florian Bach
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

// This is a modified and stripped-down version of the apploader code in 
// apploader.c. It runs before the main apploader, and is responsible for
// initializing the disc drive and figuring out what IOS the inserted
// game needs to run. That information (the IOS number) is then returned 
// to the main program, so a reload to that particular IOS can be performed.
// After that, the full apploader in apploader.c takes over and loads
// the game. 

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



event_t apploader_event_disc_loading;
event_t apploader_event_got_ios;
event_t apploader_event_got_disc_id;

int _apploader_game_ios = 0;

#define APPLOADER_APP0_BOUNDARY ((void *)0x81200000)
#define APPLOADER_APP1_BOUNDARY ((void *)0x81400000)

u32 apploader_ipc_tmd[0x4A00 / 4] ATTRIBUTE_ALIGN(32);

static void *IOSApploader_Main(void *arg);

bool IOSApploader_Init(void) {
    return 
        Event_Init(&apploader_event_got_ios) && 
        Event_Init(&apploader_event_got_disc_id) && 
        Event_Init(&apploader_event_disc_loading) ;
}

bool IOSApploader_RunBackground(void) {
    int ret;
    lwp_t thread;
    
    ret = LWP_CreateThread(
        &thread, &IOSApploader_Main,
        NULL, NULL, 0, THREAD_PRIO_IO);
        
    if (ret) {
        errno = ENOMEM;
        return false;
    }
    
    return true;
}

void IOSApploader_Report(const char *format, ...) {
#if 0
    /* debugging code, uncomment to display apploader logging messages */
    va_list args;

    va_start(args, format);
    vprintf(message, sizeof(message), format, args);
    va_end(args);
#endif
}
    
partition_info_t *boot_partition;
partition_info_t ipc_partition_info[4] ATTRIBUTE_ALIGN(32);
contents_t ipc_toc[4] ATTRIBUTE_ALIGN(32);


static void *IOSApploader_Main(void *arg) {
    int ret, i;
    
    do {
        ret = DI_Init();
    } while (ret);
    
    tryagain: 

    // wait until a disc is inserted, 
    // then trigger the loading event, 
    // then ... reset? Why?
    while ((ret = DI_DiscInserted()) != 1) {
        if (ret < 0) {
            continue;
        }
        ;   // wait for disc to be inserted. 
    }

    Event_Trigger(&apploader_event_disc_loading);

    do { 
        ret = DI_Only_Reset(); 
    } while (ret); 
    
    do {
        ret = DI_ReadDiscID();
        if (ret) { 
            if (DI_DiscInserted() != 1) {
                // If the disc's missing, start over
                usleep(2000);
                goto tryagain;
            }
            usleep(2000);
        }
    } while (ret);
        
    Event_Trigger(&apploader_event_got_disc_id);
    
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
    
    // Get IOS information
    {
        tmd *dvd_tmd;
        
        dvd_tmd = SIGNATURE_PAYLOAD(apploader_ipc_tmd);
        
        /* 
        printf(
            "Title ID: %08x-%.4s\nIOS: %08x-IOS%d\n",
            (int)(dvd_tmd->title_id >> 32), (char *)&dvd_tmd->title_id + 4,
            (int)(dvd_tmd->sys_version >> 32), (int)dvd_tmd->sys_version);
        */

        _apploader_game_ios = (int)dvd_tmd->sys_version;

    }

    // cleanup and close partition again so we don't have to re-init everything. 
    do {
        ret = DI_PartitionClose();
    } while (ret < 0);

    Event_Trigger(&apploader_event_got_ios);


    
    return NULL;
}
