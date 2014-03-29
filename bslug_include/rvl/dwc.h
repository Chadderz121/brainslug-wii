/* rvl/dwc.h
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

/* definitions of symbols inferred to exist in a dwc header file for which the
 * brainslug symbol information is available. */
 
#ifndef _RVL_DWC_H_
#define _RVL_DWC_H_

/* DWC is the online matchmaking library. */
typedef enum DWC_LogType_t {
    DWC_Info = 1 << 0,
    DWC_Error = 1 << 1,
    DWC_Debug = 1 << 2,
    DWC_Warn = 1 << 3,
    DWC_Acheck = 1 << 4,
    DWC_Login = 1 << 5,
    DWC_Match_NN = 1 << 6,
    DWC_Match_GT2 = 1 << 7,
    DWC_Transport = 1 << 8,
    DWC_QR2_Req = 1 << 9,
    DWC_SB_Update = 1 << 10,
    DWC_Send = 1 << 15,
    DWC_Recv = 1 << 16,
    DWC_Update_SV = 1 << 17,
    DWC_ConnectInet = 1 << 18,
    DWC_Auth = 1 << 24,
    DWC_Ac = 1 << 25,
    DWC_Bm = 1 << 26,
    DWC_Util = 1 << 27,
    DWC_Option_CF = 1 << 28,
    DWC_Option_ConnTest = 1 << 29,
    DWC_Gamespy = 1 << 31,
    DWC_Unknown,
} DWC_LogType_t;

#ifdef __GNUC__
#ifndef __ATTRIBUTE_PRINTF
#define __ATTRIBUTE_PRINTF(str, args) \
    __attribute__ ((format (printf, str, args)))
#endif
#endif

/* or together bits in DWC_LogType_t to set which messages to allow. */
void DWC_SetLogMask(DWC_LogType_t log_mask);
void DWC_Log(DWC_LogType_t category, const char *format, ...)
    __ATTRIBUTE_PRINTF(2, 3);

#endif /* _RVL_DWC_H_ */