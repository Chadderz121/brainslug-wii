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
#include <errno.h>
#include <io/fat-sd.h>
#include <io/libsd.h>
#include <rvl/OSMutex.h>
#include <stdbool.h>

BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("libfat-sd");
BSLUG_MODULE_VERSION("v1.0");
BSLUG_MODULE_AUTHOR("Chadderz");
BSLUG_MODULE_LICENSE("BSD");

BSLUG_EXPORT(sd_partition);
PARTITION sd_partition;


BSLUG_EXPORT(SD_Mount);
int SD_Mount(void) {
    static uint8_t sd_cache[512 * 8 * 64];
    static OSMutex_t init_mutex;
    static bool hasInit = false;
    
    if (hasInit)
        return 0;
        
    OSLockMutex(&init_mutex);
    if (hasInit) {
        OSUnlockMutex(&init_mutex);
        return 0;
    }
    if (init_mutex.lock_count > 1) {
        /* we have a cycle somewhere in the modules; abort this attempt. */
        OSUnlockMutex(&init_mutex);
        errno = EDEADLK;
        return -1;
    }
    
    if (!__io_wiisd.startup())
        goto exit_error;
    
    if (FAT_partition_constructor(
        &__io_wiisd, &sd_partition, sd_cache, sizeof(sd_cache), 0) == NULL) {
     
        __io_wiisd.shutdown();
        goto exit_error;
    }
    
    hasInit = true;

exit_error:
    OSUnlockMutex(&init_mutex);
    
    if (hasInit)
        return 0;
    return -1;
}
