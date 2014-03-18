/* di.c
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

#include "di.h"

#include <assert.h>
#include <errno.h>
#include <ogc/es.h>
#include <ogc/ipc.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/unistd.h>

#define DI_IOCTL_IDENTIFY           0x12
#define DI_IOCTL_READ_DISCID        0x70
#define	DI_IOCTL_READ               0x71
#define DI_IOCTL_WAITFORCOVERCLOSE  0x79
#define DI_IOCTL_GETCOVER           0x88
#define DI_IOCTL_RESET              0x8A
#define	DI_IOCTLV_OPEN_PARTITION    0x8B
#define	DI_IOCTL_CLOSE_PARTITION    0x8C
#define DI_IOCTL_READ_UNENCRYPTED   0x8D
#define	DI_IOCTL_SET_MOTOR          0xE3

static char di_ipc_path[] ATTRIBUTE_ALIGN(32) = "/dev/di";
static uint32_t di_ipc_in[8] ATTRIBUTE_ALIGN(32);
static uint32_t di_ipc_out[8] ATTRIBUTE_ALIGN(32);
static ioctlv di_ipc_iovector[5] ATTRIBUTE_ALIGN(32);
static int di_fd
#ifndef NDEBUG
    = -1
#endif
    ;
#ifndef NDEBUG
static bool di_has_partition = false;
#endif
    
static inline int IOS_Errno(int ret) {
    switch (ret) {
        case IPC_OK:
            return 0;
        case IPC_EINVAL:
            return EINVAL;
        case IPC_ENOHEAP:
            return E2BIG;
        case IPC_ENOENT:
            return ENOENT;
        case IPC_EQUEUEFULL:
            return EAGAIN;
        case IPC_ENOMEM:
            return ENOMEM;
    }
    return EIO;
}

int DI_Init(void) {
    int ret;
    
    assert(di_fd == -1);
    
    ret = IOS_Open(di_ipc_path, 0);
    if (ret == -1) {
        errno = EIO;
        return -1;
    }
    di_fd = ret;
    
    return 0;
}

int DI_Close(void) {
    int ret;
    
    assert(di_fd != -1);
    
    ret = IOS_Close(di_fd);
    
    if (ret) {
        errno = IOS_Errno(ret);
        return -1;
    }
#ifndef NDEBUG
    di_fd = -1;
#endif
    return 0;
}

int DI_Read(void *buffer, uint32_t length, uint32_t offset) {
    int ret;
    
    assert(di_fd != -1);
    assert(buffer);
    assert(di_has_partition);
    assert(((int)buffer & 0x1f) == 0);

    di_ipc_in[0] = DI_IOCTL_READ << 24;
    di_ipc_in[1] = length;
    di_ipc_in[2] = offset;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_READ,
        di_ipc_in, sizeof(di_ipc_in),
        buffer, length);
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
    return ret;
}

int DI_DiscWait(void) {
    int ret;
    
    assert(di_fd != -1);

    di_ipc_in[0] = DI_IOCTL_WAITFORCOVERCLOSE << 24;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_WAITFORCOVERCLOSE,
        di_ipc_in, sizeof(di_ipc_in),
        NULL, 0);
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
    return ret;
}

int DI_DiscInserted(void) {
    int ret;
    
    assert(di_fd != -1);

    di_ipc_in[0] = DI_IOCTL_GETCOVER << 24;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_GETCOVER,
        di_ipc_in, sizeof(di_ipc_in),
        di_ipc_out, sizeof(di_ipc_out));
    
    if (ret != 1) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
    if (di_ipc_out[0] & 2)
        return 1;
    else
        return 0;
}

int DI_Reset(void) {
    int ret, attempt;
    
    assert(di_fd != -1);
    
    for (attempt = 0; attempt < 10; attempt++) {
        // ensure this loop isn't too aggressive
        if (attempt > 0)
            sleep(2);
        
        ret = DI_DiscInserted();
        if (ret < 0)
            continue; // try again
        if (!ret) {// we don't have disc
            ret = DI_DiscWait();
            if (ret != 4)
                continue; // try again
        }
        
        di_ipc_in[0] = DI_IOCTL_RESET << 24;
        di_ipc_in[1] = 1;
        ret = IOS_Ioctl(
            di_fd, DI_IOCTL_RESET,
            di_ipc_in, sizeof(di_ipc_in),
            NULL, 0);
        
        if (ret < 0) {
            errno = IOS_Errno(ret);
            continue; // try again
        }
        if (ret != 1)
            continue; // try again
        
        di_ipc_in[0] = DI_IOCTL_IDENTIFY << 24;
        ret = IOS_Ioctl(
            di_fd, DI_IOCTL_IDENTIFY,
            di_ipc_in, sizeof(di_ipc_in),
            di_ipc_out, sizeof(di_ipc_out));
        
        if (ret < 0) {
            errno = IOS_Errno(ret);
            continue; // try again
        }
        if (ret != 1)
            continue; // try again
        
        di_ipc_in[0] = DI_IOCTL_READ_DISCID << 24;
        ret = IOS_Ioctl(
            di_fd, DI_IOCTL_READ_DISCID,
            di_ipc_in, sizeof(di_ipc_in),
            (void *)0x80000000, 0x20);
        
        if (ret < 0) {
            errno = IOS_Errno(ret);
            continue; // try again
        }
        if (ret != 1)
            continue; // try again
            
        break;
    }
    
    if (attempt == 10)
        return -1;
    
    return 0;
}

int DI_PartitionOpen(uint32_t offset, signed_blob *tmd) {
    int ret;
    
    assert(di_fd != -1);
    assert(!di_has_partition);

    di_ipc_in[0] = DI_IOCTLV_OPEN_PARTITION << 24;
    di_ipc_in[1] = offset;
    
    di_ipc_iovector[0].data = di_ipc_in;
    di_ipc_iovector[0].len = sizeof(di_ipc_in);
    di_ipc_iovector[1].data = NULL;
    di_ipc_iovector[1].len = 0;
    di_ipc_iovector[2].data = NULL;
    di_ipc_iovector[2].len = 0;
    di_ipc_iovector[3].data = tmd;
    di_ipc_iovector[3].len = 18916;
    di_ipc_iovector[4].data = di_ipc_out;
    di_ipc_iovector[4].len = sizeof(di_ipc_out);
    ret = IOS_Ioctlv(di_fd, DI_IOCTLV_OPEN_PARTITION, 3, 2, di_ipc_iovector);
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
#ifndef NDEBUG
    di_has_partition = true;
#endif
    
    return ret;
}

int DI_PartitionClose(void) {
    int ret;
    
    assert(di_fd != -1);
    assert(di_has_partition);

    di_ipc_in[0] = DI_IOCTL_CLOSE_PARTITION << 24;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_CLOSE_PARTITION,
        di_ipc_in, sizeof(di_ipc_in),
        NULL, 0);
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
#ifndef NDEBUG
    di_has_partition = false;
#endif
    
    return ret;
}

int DI_ReadUnencrypted(void *buffer, uint32_t length, uint32_t offset) {
    int ret;
    
    assert(di_fd != -1);
    assert(buffer);
    assert(((int)buffer & 0x1f) == 0);

    di_ipc_in[0] = DI_IOCTL_READ_UNENCRYPTED << 24;
    di_ipc_in[1] = length;
    di_ipc_in[2] = offset;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_READ_UNENCRYPTED,
        di_ipc_in, sizeof(di_ipc_in),
        buffer, length);
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
    return ret;
}

int DI_MotorStop(void) {
    int ret;
    
    assert(di_fd != -1);

    di_ipc_in[0] = DI_IOCTL_SET_MOTOR << 24;
    di_ipc_in[1] = 0;
    di_ipc_in[2] = 0;
    ret = IOS_Ioctl(
        di_fd, DI_IOCTL_SET_MOTOR,
        di_ipc_in, sizeof(di_ipc_in),
        di_ipc_out, sizeof(di_ipc_out));
    
    if (ret < 0) {
        errno = IOS_Errno(ret);
        return -1;
    }
    
    return ret;    
}