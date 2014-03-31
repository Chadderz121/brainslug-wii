/* vi.h
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

/* definitions of symbols inferred to exist in the vi header file for
 * which the brainslug symbol information is available. */

#ifndef _RVL_VI_H_
#define _RVL_VI_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum VIVideoMode_t {
    VI_MODE_INTERLACE,
    VI_MODE_NON_INTERLACE,
    VI_PROGRESSIVE
} VIVideoMode_t;

typedef enum VIStandard_t {
    VI_NTSC,      /* North America and Japan */
    VI_PAL,       /* Europe */
    VI_MPAL,      /* Similar to NTSC, used in Brazil */
    VI_DEBUG,     /* Debug for North America and Japan. Special decoder needed */
    VI_DEBUG_PAL, /* Debug for Europe. Special decoder needed */
    VI_EURGB60    /* RGB 60Hz used in Europe */
} VIStandard_t;

typedef struct VITVMode_t {
    VIVideoMode_t mode : 2;
    VIStandard_t  standard : 30;
} VITVMode_t;

typedef enum VIXFBMode_t {
    VI_XFBMODE_SF,
    VI_XFBMODE_DF
} VIXFBMode_t;

typedef struct GXRenderModeObj {
	VITVMode_t viTVMode; /* 0x0 mode and type of TV */
	uint16_t fbWidth; /* 0x4 width of external framebuffer */
	uint16_t efbHeight; /* 0x6 height of embedded framebuffer */
	uint16_t xfbHeight; /* 0x8 height of external framebuffer */
	uint16_t viXOrigin; /* 0xa x starting point of first pixel to draw on VI */
	uint16_t viYOrigin; /* 0xc y starting point of first pixel to draw on VI */
	uint16_t viWidth; /* 0xe width of configured VI */
	uint16_t viHeight; /* 0x10 height of configured VI */
    /* 0x12 implicit padding */
	VIXFBMode_t xfbMode; /* 0x14 */
	uint8_t  field_rendering; /* 0x18 */
	uint8_t  aa; /* 0x19 */
	uint8_t  sample_pattern[12][2]; /* 0x1a */
	uint8_t  vfilter[7]; /* 0x32 */
    /* 0x39 implicit padding */    
} GXRenderModeObj;

void VIInit(void);
void VIWaitForRetrace(void);
void VIConfigure(const GXRenderModeObj *mode);
void VIConfigurePan(int x, int y, int width, int height);
void VIFlush(void);
void VISetNextFrameBuffer(void *frame_buffer);
void VISetBlack(bool black);

#endif /* _RVL_VI_H_ */