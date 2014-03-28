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

#include <rvl/GXTransform.h>

/* support any game id */
BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("GX Console");
BSLUG_MODULE_VERSION("v1.0");
BSLUG_MODULE_AUTHOR("Chadderz");
BSLUG_MODULE_LICENSE("BSD");

static void Console_GXSetViewport(
    void *viewport, float x, float y, float width, float height,
    float z, float depth);
static void Console_GXSetScissor(
    unsigned int x, unsigned int y, unsigned int width, unsigned int height);

BSLUG_REPLACE(GXSetViewport, Console_GXSetViewport);
BSLUG_REPLACE(GXSetScissor, Console_GXSetScissor);

static void Console_GXSetViewport(
    void *viewport, float x, float y, float width, float height,
    float z, float depth) {

    if (width >= 608.0f) {
        width /= 2;
        x /= 2;
    }
    
    GXSetViewport(viewport, x, y, width, height, z, depth);
}
static void Console_GXSetScissor(
    unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    
    if (width >= 608) {
        width = 304;
        x /= 2;
    }
    
    GXSetScissor(x, y, width, height);
}
