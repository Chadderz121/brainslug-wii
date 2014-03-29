/* GXFrameBuf.h
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

/* definitions of symbols inferred to exist in the GXFrameBuf header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_GXFRAMEBUF_H_
#define _RVL_GXFRAMEBUF_H_

enum {
    GX_COPY_PROGRESSIVE = 0,
    GX_COPY_INTLC_EVEN = 2,
    GX_COPY_INTLC_ODD = 3,
};

void GXSetDispCopyDst(unsigned short width, unsigned short height);
void GXSetDispCopyFrame2Field(int mode);
float GXGetYScaleFactor(unsigned short efb_height, unsigned short xfb_height);
void GXSetDispCopyYScale(float factor);

#endif /* _RVL_GXTRANSFORM_H_ */