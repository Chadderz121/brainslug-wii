/*
	fat.h
	Simple functionality for startup, mounting and unmounting of FAT-based devices.
	
 Copyright (c) 2006 - 2014
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
    Alex "Chadderz" Chadwick

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _LIBFAT_H
#define _LIBFAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libfatversion.h"

#include <io/disc_io.h>
#include <rvl/OSMutex.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>

/* A note on file paths:
 *  / is the root directory.
 *  Device names like sd:/ will be ignored. */ 

typedef struct PARTITION PARTITION;
typedef struct _FILE_STRUCT FILE_STRUCT;
typedef struct DIR_STATE_STRUCT DIR_STATE_STRUCT;
typedef enum FILE_ATTR FILE_ATTR;

/******************************************************************************/
/* Partition handling */

/* Mount the supplied device's partition.
 *  disc        - device to mount
 *  partition   - buffer to fill partition data in
 *  cacheSpace  - buffer to create cache in (at least 16kb)
 *  cacheSize   - size of cacheSpace buffer
 *  startSector - first sector of partition [0 unless you know otherwise]
 * Returns NULL on error. */
PARTITION *FAT_partition_constructor(
    const DISC_INTERFACE* disc, PARTITION *partition, uint8_t *cacheSpace,
    size_t cacheSize, sec_t startSector);
/* Dismount the device and synchronise all open files to disc. */
void FAT_partition_destructor(PARTITION* partition);

/******************************************************************************/
/* File handling */

/* Open a file.
 *  fileStruct - buffer to store file information in.
 *  partition  - partition to open file in.
 *  path       - path of file.
 *  flags      - open flags from <fcntl.h> O_RDONLY, O_RDWR, O_WRONLY, O_CREAT,
 *               O_APPEND, O_TRUNC
 * Returns -1 on error or a fd for use in other file calls. Sets errno. */
int FAT_open(
    FILE_STRUCT *fileStruct, PARTITION *partition, const char *path, int flags);
/* Close a file and synch to device. Returns 0 on success. */
int FAT_close(int fd);
/* Write to a file.
 * Returns number of bytes successfully written (or -1 on error). Sets errno. */
ssize_t FAT_write(int fd, const char *ptr, size_t len);
/* Read from a file.
 * Returns number of bytes successfully read (or -1 on error). Sets errno. */
ssize_t FAT_read(int fd, char *ptr, size_t len);
/* Seek within file. dir is one of SEEK_SET, SEEK_CUR or SEEK_END (stdio.h).
 * Returns new absolute position (or -1 on error). Sets errno. */
off_t FAT_seek(int fd, off_t pos, int dir);

/* Shortens (or expands) a file to specified length.
 * Returns 0 on success. Sets errno. */
int FAT_ftruncate(int fd, off_t len);
/* Synchronises a file to the device. This call does not return until all
 * prior write calls to this file have been sent to the device.
 * Returns 0 on success. Sets errno. */
int FAT_fsync(int fd);
/* Fills in the st buffer with metadata about the file.
 * Returns 0 on success. Sets errno. */
int FAT_fstat(int fd, struct stat *st);

/******************************************************************************/
/* File system manipulation */

enum FILE_ATTR {
    ATTR_ARCHIVE	= 0x20,			/* Archive */
    ATTR_DIRECTORY	= 0x10,			/* Directory */
    ATTR_VOLUME		= 0x08,			/* Volume */
    ATTR_SYSTEM		= 0x04,			/* System */
    ATTR_HIDDEN		= 0x02,			/* Hidden */
    ATTR_READONLY	= 0x01			/* Read only */
};

/* Gets the DOS file attributes of the specified file.
 * Returns -1 on error. Does not set errno. */
FILE_ATTR FAT_getAttr(PARTITION *partition, const char *file);
/* Sets the DOS file attributes of the specified file.
 * Returns 0 on success. Does not set errno. */
int FAT_setAttr(PARTITION *partition, const char *file, FILE_ATTR attr);

/* Fills in the st buffer with metadata about the specified file.
 * Returns 0 on success. Sets errno. */
int FAT_stat(PARTITION *partition, const char *path, struct stat *st);
/* Fills in the st buffer with metadata about the specified filesystem.
 * Returns 0 on success. Sets errno. */
int FAT_statvfs(PARTITION *partition, const char *path, struct statvfs *buf);

/* Deletes the specified file.
 * Returns 0 on success. Sets errno. */
int FAT_unlink(PARTITION *partition, const char *name);
/* Renames (or moves) a file from one name to another.
 * Returns 0 on success. Sets errno. */
int FAT_rename(PARTITION *partition, const char *oldName, const char *newName);

/* Sets the path for all future relative file paths.
 * Returns 0 on success. Sets errno. */
int FAT_chdir(PARTITION *partition, const char *name);

/* Creates the specified directory.
 * Returns 0 on success. Sets errno. */
int FAT_mkdir(PARTITION *partition, const char *path);

/******************************************************************************/
/* Directory listing */

/* Opens a directory for listing purposes.
 *  state     - buffer in which to store directory information.
 *  partition - partition on which the directory resides.
 *  path      - path to the directory.
 * Returns NULL on error. Sets errno. */
DIR_STATE_STRUCT* FAT_diropen(
    DIR_STATE_STRUCT *state, PARTITION *partition, const char *path);
/* Resets a directory iterator.
 * Returns 0 on success. Sets errno. */
int FAT_dirreset(DIR_STATE_STRUCT *state);
/* Moves to the next file in a directory.
 *  state    - iterator to advance
 *  filename - buffer in which to store name of next file. Will not exceed
 *             MAX_FILENAME_LENGTH.
 *  filestat - buffer in which to store metadata about the next file.
 * Returns 0 on success. Sets errno.
 * Returns -1 and sets errno to ENOENT if no more files avilable. */
int FAT_dirnext(DIR_STATE_STRUCT *state, char *filename, struct stat *filestat);
/* Closes a directory iterator.
 * Returns 0 on success. Sets errno. */
int FAT_dirclose(DIR_STATE_STRUCT *state);

/******************************************************************************/
/* Avoid directly reading or modifying any of these structures. */

#define LIBFAT_FEOS_MULTICWD

// Filesystem type
typedef enum {FS_UNKNOWN, FS_FAT12, FS_FAT16, FS_FAT32} FS_TYPE;

typedef struct {
	sec_t    fatStart;
	uint32_t sectorsPerFat;
	uint32_t lastCluster;
	uint32_t firstFree;
	uint32_t numberFreeCluster;
	uint32_t numberLastAllocCluster;
} FAT;

struct CACHE;
struct _FILE_STRUCT;

struct PARTITION {
	const DISC_INTERFACE* disc;
	struct CACHE*         cache;
	// Info about the partition
	FS_TYPE               filesysType;
	uint64_t              totalSize;
	sec_t                 rootDirStart;
	uint32_t              rootDirCluster;
	uint32_t              numberOfSectors;
	sec_t                 dataStart;
	uint32_t              bytesPerSector;
	uint32_t              sectorsPerCluster;
	uint32_t              bytesPerCluster;
	uint32_t              fsInfoSector;
	FAT                   fat;
	// Values that may change after construction
	uint32_t              cwdCluster;			// Current working directory cluster
	int                   openFileCount;
	struct _FILE_STRUCT*  firstOpenFile;		// The start of a linked list of files
	OSMutex_t             lock;					// A lock for partition operations
	bool                  readOnly;				// If this is set, then do not try writing to the disc
	char                  label[12];			// Volume label
};

#define FILE_MAX_SIZE ((uint32_t)0xFFFFFFFF)	// 4GiB - 1B

typedef struct {
	uint32_t cluster;
	sec_t sector;
	int32_t byte;
} FILE_POSITION;

typedef struct {
	uint32_t cluster;
	sec_t    sector;
	int32_t  offset;
} DIR_ENTRY_POSITION;

struct _FILE_STRUCT {
	uint32_t             filesize;
	uint32_t             startCluster;
	uint32_t             currentPosition;
	FILE_POSITION        rwPosition;
	FILE_POSITION        appendPosition;
	DIR_ENTRY_POSITION   dirEntryStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION   dirEntryEnd;		// Always points to the file's alias entry
	PARTITION*           partition;
	struct _FILE_STRUCT* prevOpenFile;		// The previous entry in a double-linked list of open files
	struct _FILE_STRUCT* nextOpenFile;		// The next entry in a double-linked list of open files
	bool                 read;
	bool                 write;
	bool                 append;
	bool                 inUse;
	bool                 modified;
};

typedef enum {FT_DIRECTORY, FT_FILE} FILE_TYPE;

#define DIR_ENTRY_DATA_SIZE 0x20
#define MAX_FILENAME_LENGTH 768		// 256 UCS-2 characters encoded into UTF-8 can use up to 768 UTF-8 chars

typedef struct {
	uint8_t            entryData[DIR_ENTRY_DATA_SIZE];
	DIR_ENTRY_POSITION dataStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION dataEnd;			// Always points to the file/directory's alias entry
	char               filename[MAX_FILENAME_LENGTH];
} DIR_ENTRY;

typedef struct DIR_STATE_STRUCT {
	PARTITION* partition;
	DIR_ENTRY  currentEntry;
	uint32_t   startCluster;
	bool       inUse;
	bool       validEntry;
} DIR_STATE_STRUCT;

#ifdef __cplusplus
}
#endif

#endif // _LIBFAT_H
