/* rvl/ipc.h
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

/* definitions of standard symbols typically in the ogc/ipc.h header file for
 * which the brainslug symbol information is available. */
 
#ifndef _RVL_IPC_H_
#define _RVL_IPC_H_

#include <stddef.h> /* for size_t */

#define IPC_MAXPATH_LEN 64

typedef enum {
    IPC_OK = 0,
    IPC_EBUSY = -2,
    IPC_EINVAL = -4,
    IPC_ENOENT = -6,
    IPC_EQUEUEFULL = -8,
    IPC_ENOMEM = -22,
} ios_ret_t;

typedef enum {
    IPC_OPEN_NONE = 0,
    IPC_OPEN_READ = 1,
    IPC_OPEN_WRITE = 2,
    IPC_OPEN_RW = IPC_OPEN_READ + IPC_OPEN_WRITE
} ios_mode_t;

typedef void *usr_t;
typedef int ios_fd_t;
typedef void (*ios_open_cb_t)(ios_fd_t result, usr_t usrdata);
typedef void (*ios_cb_t)(ios_ret_t result, usr_t usrdata);

typedef struct _ioctlv {
	void *data;
	size_t len;
} ioctlv;

ios_fd_t IOS_Open(const char *filepath, ios_mode_t mode);
ios_ret_t IOS_OpenAsync(
    const char *filepath, ios_mode_t mode, ios_open_cb_t cb, usr_t usrdata);
    
ios_ret_t IOS_Close(ios_fd_t fd);
ios_ret_t IOS_CloseAsync(ios_fd_t fd, ios_cb_t cb, usr_t usrdata);

ios_ret_t IOS_Read(ios_fd_t fd, void *buffer, size_t length);
ios_ret_t IOS_ReadAsync(
    ios_fd_t fd, void *buffer, size_t length, ios_cb_t cb, usr_t usrdata);
ios_ret_t IOS_Write(ios_fd_t fd, const void *buffer, size_t length);
ios_ret_t IOS_WriteAsync(
    ios_fd_t fd, const void *buffer, size_t length, ios_cb_t cb, usr_t usrdata);
ios_ret_t IOS_Seek(ios_fd_t fd, int offset, int base);
ios_ret_t IOS_SeekAsync(
    ios_fd_t fd, int offset, int base, ios_cb_t cb, usr_t usrdata);

ios_ret_t IOS_Ioctl(
    ios_fd_t fd, int ioctl, const void *input, size_t input_length,
    void *output, size_t output_length);
ios_ret_t IOS_IoctlAsync(
    ios_fd_t fd, int ioctl, const void *input, size_t input_length,
    void *output, size_t output_length, ios_cb_t cb, usr_t usrdata);

ios_ret_t IOS_Ioctlv(
    ios_fd_t fd, int ioctl, int input_count, int output_count, ioctlv *argv);
ios_ret_t IOS_IoctlvAsync(
    ios_fd_t fd, int ioctl, int input_count, int output_count, ioctlv *argv,
    ios_cb_t cb, usr_t usrdata);
ios_ret_t IOS_IoctlvReboot(
    ios_fd_t fd, int ioctl, int input_count, int output_count, ioctlv *argv);

typedef int hid_t;

/* space must be aligned to 32 */
hid_t iosCreateHeap(void *space, size_t size);
void *iosAllocAligned(hid_t hid, size_t size, size_t alignment);
static inline void *iosAlloc(hid_t hid, size_t size);
void iosFree(hid_t hid, void *ptr);

static inline void *iosAlloc(hid_t hid, size_t size) {
    return iosAllocAligned(hid, size, 32);
}
    
#endif /* _RVL_IPC_H_ */