/* main.c
 *   by Michael Lelli
 * 
 * Copyright (C) 2014, Michael Lelli
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
#include <rvl/cache.h>
#include <string.h>

BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("HTTPS to HTTP");
BSLUG_MODULE_VERSION("v1.0");
BSLUG_MODULE_AUTHOR("Toad King");
BSLUG_MODULE_LICENSE("BSD");

void _start(void);

static void _start_https(void)
{
   char *cur = (char *)bslug_game_start;
   const char *end = (char *)bslug_game_end;

   do
   {
      if (memcmp(cur, "https://", 8) == 0 && cur[8] != 0)
      {
         int len = strlen(cur);
         memmove(cur + 4, cur + 5, len - 5);
         cur[len - 1] = 0;
         DCFlushRange((void *)((unsigned) cur & ~0x1F), ((unsigned) len + 0x3F) & ~0x1F);
         cur += len;
      }
   } while (++cur < end);

   _start();
}

BSLUG_MUST_REPLACE(_start, _start_https);
