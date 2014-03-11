/* dolphin_os.h
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

#ifndef DOLPHIN_OS_H_
#define DOLPHIN_OS_H_

#include <stdint.h>

typedef struct {
	char gamename[4];
	char company[2];
	uint8_t disknum;
	uint8_t gamever;
	uint8_t streaming;
	uint8_t streambufsize;
	uint8_t pad[14];
    uint32_t wii_magic;
    uint32_t gc_magic;
} os_disc_id_t;

typedef enum {
    OS_BOOT_NORMAL = 0x0d15ea5e
} os_boot_type_t;

typedef struct {
    uint32_t boot_type;
    uint32_t version;
    uint32_t mem1_size;
    uint32_t console_type;
    uint32_t arena_low;
    uint32_t arena_high;
    void *fst;
    uint32_t fst_size;
} os_system_info_t;

typedef struct {
    uint32_t enabled;
    uint32_t exception_mask;
    void *destination;
    uint8_t temp[0x14];
    uint8_t hook[0x24];
    uint8_t padding[0x3c];
} os_debugger_t;

typedef enum {
    OS_TV_MODE_NTSC,
    OS_TV_MODE_PAL,
    OS_TV_MODE_DEBUG,
    OS_TV_MODE_DEBUG_PAL,
    OS_TV_MODE_MPAL,
    OS_TV_MODE_PAL60,
} os_tv_mode_t;

typedef struct {
    void *current_context_phy;
    uint32_t previous_interrupt_mask;
    uint32_t current_interrupt_mask;
    uint32_t tv_mode;
    uint32_t aram_size;
    void *current_context;
    void *default_thread;
    void *thread_queue_head;
    void *thread_queue_tail;
    void *current_thread;
    uint32_t debug_monitor_size;
    void *debug_monitor_location;
    uint32_t simulated_memory_size;
    void *bi2;
    uint32_t bus_speed;
    uint32_t cpu_speed;
} os_thread_info_t;

typedef struct {
    os_disc_id_t disc; /* 0x0 */
    os_system_info_t info; /* 0x20 */
    os_debugger_t debugger; /* 0x40 */
    os_thread_info_t threads; /* 0xc0 */
} os_early_globals_t;

os_early_globals_t * const os0 = (os_early_globals_t *)0x80000000;

typedef struct {
    uint8_t padding0[0x100];
    uint32_t mem1_size;
    uint32_t mem1_simulated_size;
    uint8_t padding108[0x8];
    void *fst;
    uint8_t padding10c[0x4];
    uint32_t mem2_size;
    uint32_t mem2_simulated_size;
    uint8_t padding120[0x10];
    uint32_t ios_heap_start;
    uint32_t ios_heap_end;
    uint32_t hollywood_version;
    uint16_t ios_number;
    uint16_t ios_revision;
    uint32_t ios_build_date;
    uint8_t padding148[0x10];
    uint32_t gddr_vendor_id;
    uint32_t legacy_di;
    uint32_t init_semaphore;
    uint32_t mios_flag;
    uint8_t padding168[0x18];
    char application_name[4];
    os_disc_id_t *id;
    uint16_t expected_ios_number;
    uint16_t expected_ios_revision;
    uint32_t launch_code;
    uint32_t return_code;
} os_late_globals_t;

os_late_globals_t * const os1 = (os_late_globals_t *)0x80003000;

#endif /* DOLPHIN_OS_H_ */