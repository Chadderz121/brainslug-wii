/* main.c
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

#include "main.h"

#include <fat.h>
#include <malloc.h>
#include <ogc/consol.h>
#include <ogc/lwp.h>
#include <ogc/system.h>
#include <ogc/video.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <stdlib.h>

#include "apploader/apploader.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "modules/module.h"
#include "threads.h"

event_t main_event_fat_loaded;

static void Main_PrintSize(size_t size);

int main(void) {
    int ret;
    void *frame_buffer = NULL;
    GXRModeObj *rmode = NULL;
    
    /* The game's boot loader is statically loaded at 0x81200000, so we'd better
     * not start mallocing there! */
    SYS_SetArena1Hi((void *)0x81200000);

    /* initialise all subsystems */
    if (!Event_Init(&main_event_fat_loaded))
        goto exit_error;
    if (!Apploader_Init())
        goto exit_error;
    if (!Module_Init())
        goto exit_error;
    
    /* main thread is UI, so set thread prior to UI */
    LWP_SetThreadPriority(LWP_GetSelf(), THREAD_PRIO_UI);

    /* spawn lots of worker threads to do stuff */
    if (!Apploader_RunBackground())
        goto exit_error;
    if (!Module_RunBackground())
        goto exit_error;

    /* configure the video */
    VIDEO_Init();
    
    rmode = VIDEO_GetPreferredMode(NULL);
    
    frame_buffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    if (!frame_buffer)
        goto exit_error;
    console_init(
        frame_buffer, 20, 20, rmode->fbWidth, rmode->xfbHeight,
        rmode->fbWidth * VI_DISPLAY_PIX_SZ);
        
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frame_buffer);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    /* display the welcome message */
    printf("\x1b[2;0H");
    printf("BrainSlug Wii  v%x.%02x.%04x"
#ifndef NDEBUG
        " DEBUG build"
#endif
        "\n",
        BSLUG_VERSION_MAJOR(BSLUG_LOADER_VERSION),
        BSLUG_VERSION_MINOR(BSLUG_LOADER_VERSION),
        BSLUG_VERSION_REVISION(BSLUG_LOADER_VERSION));
    printf(" by Chadderz\n\n");
   
    if (!__io_wiisd.startup() || !__io_wiisd.isInserted()) {
        printf("Please insert an SD card.\n\n");
        do {
            __io_wiisd.shutdown();
        } while (!__io_wiisd.startup() || !__io_wiisd.isInserted());
    }
    __io_wiisd.shutdown();
    
    if (!fatMountSimple("sd", &__io_wiisd)) {
        fprintf(stderr, "Could not mount SD card.\n");
        goto exit_error;
    }
    
    Event_Trigger(&main_event_fat_loaded);
        
    printf("Waiting for game disk... ");
    Event_Wait(&apploader_event_disk_id);
    printf("%.4s", os0->disc.gamename);
    printf("\n");
    
    
    printf("Loading modules... ");
    Event_Wait(&module_list_loaded);
    if (module_list_count == 0) {
        printf("no valid modules found!\n");
    } else {
        size_t module;
        
        printf(
            "%u module%s found.\n",
            module_list_count, module_list_count > 1 ? "s" : "");
        
        for (module = 0; module < module_list_count; module++) {
            printf(
                "\t%s %s by %s (", module_list[module]->name,
                module_list[module]->version, module_list[module]->author);
            Main_PrintSize(module_list[module]->size);
            puts(").");
        }
        
        Main_PrintSize(module_list_size);
        puts(" total.");
    }
    
    Event_Wait(&apploader_event_complete);

    if (apploader_game_entry_fn == NULL) {
        fprintf(stderr, "Error... entry point is NULL.\n");
    } else {
        printf("\nPress RESET to launch game.\n");
        ret = 0;
        goto exit;
        
        while (!SYS_ResetButtonDown())
            VIDEO_WaitVSync();
        while (SYS_ResetButtonDown())
            VIDEO_WaitVSync();
            
        SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
        apploader_game_entry_fn();
    }

    ret = 0;
    goto exit;
exit_error:
    ret = -1;
exit:
    while (!SYS_ResetButtonDown())
        VIDEO_WaitVSync();
    while (SYS_ResetButtonDown())
        VIDEO_WaitVSync();
    
    VIDEO_SetBlack(true);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    
    free(frame_buffer);
        
    exit(ret);
        
    return ret;
}

static void Main_PrintSize(size_t size) {
    static const char *suffix[] = { "bytes", "KiB", "MiB", "GiB" };
    unsigned int magnitude, precision;
    float sizef;

    sizef = size;
    magnitude = 0;
    while (sizef > 512) {
        sizef /= 1024.0f;
        magnitude++;
    }
    
    assert(magnitude < 4);
    
    if (magnitude == 0)
        precision = 0;
    else if (sizef >= 100)
        precision = 0;
    else if (sizef >= 10)
        precision = 1;
    else
        precision = 2;
        
    printf("%.*f %s", precision, sizef, suffix[magnitude]);
}