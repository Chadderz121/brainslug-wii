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

#include <bslug.h>
#include <io/fat.h>

/* support any game id */
BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("libfat");
BSLUG_MODULE_VERSION("v1.0.12");
BSLUG_MODULE_AUTHOR("WinterMute, Chishm, Chadderz");
BSLUG_MODULE_LICENSE("LGPL");

BSLUG_EXPORT(FAT_partition_constructor);
BSLUG_EXPORT(FAT_partition_destructor);

BSLUG_EXPORT(FAT_open);
BSLUG_EXPORT(FAT_close);
BSLUG_EXPORT(FAT_write);
BSLUG_EXPORT(FAT_read);
BSLUG_EXPORT(FAT_seek);

BSLUG_EXPORT(FAT_ftruncate);
BSLUG_EXPORT(FAT_fsync);
BSLUG_EXPORT(FAT_fstat);

BSLUG_EXPORT(FAT_getAttr);
BSLUG_EXPORT(FAT_setAttr);

BSLUG_EXPORT(FAT_stat);
BSLUG_EXPORT(FAT_statvfs);

BSLUG_EXPORT(FAT_unlink);
BSLUG_EXPORT(FAT_rename);

BSLUG_EXPORT(FAT_chdir);

BSLUG_EXPORT(FAT_mkdir);

BSLUG_EXPORT(FAT_diropen);
BSLUG_EXPORT(FAT_dirreset);
BSLUG_EXPORT(FAT_dirnext);
BSLUG_EXPORT(FAT_dirclose);
