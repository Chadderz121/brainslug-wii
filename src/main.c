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
#include <gccore.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <stdlib.h>

#include "apploader/apploader.h"
#include "library/dolphin_os.h"
#include "library/event.h"
#include "modules/module.h"
#include "search/search.h"
#include "threads.h"

event_t main_event_fat_loaded;

static void Main_PrintSize(size_t size);

short current_running_ios = 0; 

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
    if (!IOSApploader_Init())
        goto exit_error;
    if (!Apploader_Init())
        goto exit_error;
    if (!Module_Init())
        goto exit_error;
    if (!Search_Init())
        goto exit_error;

    current_running_ios = *(short*)0x80003140;
    
    /* main thread is UI, so set thread prior to UI */
    LWP_SetThreadPriority(LWP_GetSelf(), THREAD_PRIO_UI);

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

    /* spawn lots of worker threads to do stuff */
    if (!IOSApploader_RunBackground())
        goto exit_error;
        
    printf("Waiting for game disk...\n");
    Event_Wait(&apploader_event_got_disc_id);
    Event_Wait(&apploader_event_got_ios);

    // Found IOS, reload into that IOS:
    printf("Game ID: %.4s on IOS%d -> reloading ... ", os0->disc.gamename, _apploader_game_ios);
    int rval = IOS_ReloadIOS(_apploader_game_ios);

    if (rval < 0) {
        // IOS reload failed, the needed IOS is probably not installed. 

        printf("\nIt looks like reloading to IOS%d failed (error %d). Maybe it is missing?\n", _apploader_game_ios, rval);
        printf("Trying to boot the game anyways (under IOS%d), but it might not work correctly.\n", current_running_ios);

        if (!Apploader_RunBackground(1))    // 1 = make the game believe it runs under the correct IOS.
            goto exit_error;

    }
    else {
        // IOS reload successful, wait for IOS to load up. 
        printf("waiting ... ");
        while (*(short*)0x80003140 != _apploader_game_ios);
        printf("done.\n");

        if (!Apploader_RunBackground(0))    // 0 = no need to fool, we are running on the correct IOS.
            goto exit_error;

    }

    // After the IOS reload, run the rest of the background threads. 
    if (!Module_RunBackground())
        goto exit_error;
    if (!Search_RunBackground())
        goto exit_error;

       
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

        
    printf("Loading modules...\n");
    Event_Wait(&module_event_list_loaded);
    if (module_list_count == 0) {
        printf("No valid modules found!\n");
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
    Event_Wait(&module_event_complete);
    fatUnmount("sd");
    __io_wiisd.shutdown();
    
    if (module_has_error) {
        printf("\nPress RESET to exit.\n");
        goto exit_error;
    }
    
    if (apploader_game_entry_fn == NULL) {
        fprintf(stderr, "Error... entry point is NULL.\n");
    } else {
        if (module_has_info || search_has_info) {
            printf("\nPress RESET to launch game.\n");
            
            while (!SYS_ResetButtonDown())
                VIDEO_WaitVSync();
            while (SYS_ResetButtonDown())
                VIDEO_WaitVSync();
        }
        
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