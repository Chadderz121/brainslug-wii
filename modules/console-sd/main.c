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

#include <bslug.h>
#include <fcntl.h>
#include <io/fat-sd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <rvl/dwc.h>
#include <rvl/OSThread.h>

/* support any game id */
BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("SD Debug Logging");
BSLUG_MODULE_VERSION("v1.0");
BSLUG_MODULE_AUTHOR("Chadderz");
BSLUG_MODULE_LICENSE("BSD");

/* Replacement for DWC_SetLogMask.
 *  always enables all logging. Calls DWC_SetLogMask with 0xffffffff. */
static void Console_DWC_SetLogMask(DWC_LogType_t log_mask);
/* Replacement for fwrite.
 *  writes messages to the the sd card. File 1 is stdout and file 2 is stderr.
 *  All other calls passed through to the real fwrite. */
static size_t Console_fwrite(
    const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* this method isn't in offline games; don't care if not patched. */
BSLUG_REPLACE(DWC_SetLogMask, Console_DWC_SetLogMask);
BSLUG_MUST_REPLACE(fwrite, Console_fwrite);

static void Console_DWC_SetLogMask(DWC_LogType_t log_mask) {
    /* call down to the real DWC_SetLogMask enabling all logging. */
    DWC_SetLogMask(0xffffffff);
}
static size_t Console_fwrite(
        const void *ptr, size_t size, size_t nmemb, FILE *stream) {    
    if (stream && ptr && (stream->fd == 1 || stream->fd == 2) && OSGetCurrentThread() != NULL) {
        /* stdout && stderr */
        static int sd_file = -1;
        
        if (sd_file == -1) {
            static FILE_STRUCT fs;
            char path[13];
            
            path[0] = 's';
            path[1] = 'd';
            path[2] = ':';
            path[3] = '/';
            path[4] = ((char *)0x80000000)[0];
            path[5] = ((char *)0x80000000)[1];
            path[6] = ((char *)0x80000000)[2];
            path[7] = ((char *)0x80000000)[3];
            path[8] = '.';
            path[9] = 'l';
            path[10] = 'o';
            path[11] = 'g';
            path[12] = '\0';
            
            if (SD_Mount() != 0)
                goto skip;
            
            sd_file = SD_open(&fs, path, O_CREAT | O_WRONLY | O_APPEND);
            if (sd_file == -1)
                goto skip;
            
            SD_write(
                sd_file, "\n\n========\n\n", sizeof("\n\n========\n\n") - 1);
        }
        
        SD_write(sd_file, ptr, size * nmemb);
        /* make sure we flush the SD regularly. */
        if (memchr(ptr, '\n', size * nmemb) != NULL) {
            SD_fsync(sd_file);
        }
skip:
        fwrite(ptr, size, nmemb, stream);
        return nmemb;
    } else
        /* call down to real fwrite. */
        return fwrite(ptr, size, nmemb, stream);
}
