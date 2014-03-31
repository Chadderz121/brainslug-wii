/* io/fat-sd.h
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

/* A simple helper-wrapper around libfat and libsd to allow mounting and using
 * of FAT formatted sd cards. */

#ifndef _IO_FAT_SD_H_
#define _IO_FAT_SD_H_

#include <io/fat.h>
#include <io/libsd.h>

/* Call this before any other methods. The first module to call SD_Mount
 * performs that actual FAT mounting procedure. Returns 0 on sucess. Sets errno.
 */
int SD_Mount(void);

/******************************************************************************/
/* File handling */

/* Open a file.
 *  fileStruct - buffer to store file information in.
 *  path       - path of file.
 *  flags      - open flags from <fcntl.h> O_RDONLY, O_RDWR, O_WRONLY, O_CREAT,
 *               O_APPEND, O_TRUNC
 * Returns -1 on error or a fd for use in other file calls. Sets errno. */
static inline int SD_open(FILE_STRUCT *fileStruct, const char *path, int flags);
/* Close a file and synch to device. Returns 0 on success. */
static inline int SD_close(int fd);
/* Write to a file.
 * Returns number of bytes successfully written (or -1 on error). Sets errno. */
static inline ssize_t SD_write(int fd, const char *ptr, size_t len);
/* Read from a file.
 * Returns number of bytes successfully read (or -1 on error). Sets errno. */
static inline ssize_t SD_read(int fd, char *ptr, size_t len);
/* Seek within file. dir is one of SEEK_SET, SEEK_CUR or SEEK_END (stdio.h).
 * Returns new absolute position (or -1 on error). Sets errno. */
static inline off_t SD_seek(int fd, off_t pos, int dir);

/* Shortens (or expands) a file to specified length.
 * Returns 0 on success. Sets errno. */
static inline int SD_ftruncate(int fd, off_t len);
/* Synchronises a file to the device. This call does not return until all
 * prior write calls to this file have been sent to the device.
 * Returns 0 on success. Sets errno. */
static inline int SD_fsync(int fd);
/* Fills in the st buffer with metadata about the file.
 * Returns 0 on success. Sets errno. */
static inline int SD_fstat(int fd, struct stat *st);

/******************************************************************************/
/* File system manipulation */

/* Gets the DOS file attributes of the specified file.
 * Returns -1 on error. Does not set errno. */
static inline FILE_ATTR SD_getAttr(const char *file);
/* Sets the DOS file attributes of the specified file.
 * Returns 0 on success. Does not set errno. */
static inline int SD_setAttr(const char *file, FILE_ATTR attr);

/* Fills in the st buffer with metadata about the specified file.
 * Returns 0 on success. Sets errno. */
static inline int SD_stat(const char *path, struct stat *st);
/* Fills in the st buffer with metadata about the specified filesystem.
 * Returns 0 on success. Sets errno. */
static inline int SD_statvfs(const char *path, struct statvfs *buf);

/* Deletes the specified file.
 * Returns 0 on success. Sets errno. */
static inline int SD_unlink(const char *name);
/* Renames (or moves) a file from one name to another.
 * Returns 0 on success. Sets errno. */
static inline int SD_rename(const char *oldName, const char *newName);

/* Sets the path for all future relative file paths.
 * Returns 0 on success. Sets errno. */
static inline int SD_chdir(const char *name);

/* Creates the specified directory.
 * Returns 0 on success. Sets errno. */
static inline int SD_mkdir(const char *path);

/******************************************************************************/
/* Directory listing */

/* Opens a directory for listing purposes.
 *  state     - buffer in which to store directory information.
 *  path      - path to the directory.
 * Returns NULL on error. Sets errno. */
static inline DIR_STATE_STRUCT* SD_diropen(
    DIR_STATE_STRUCT *state, const char *path);
/* Resets a directory iterator.
 * Returns 0 on success. Sets errno. */
static inline int SD_dirreset(DIR_STATE_STRUCT *state);
/* Moves to the next file in a directory.
 *  state    - iterator to advance
 *  filename - buffer in which to store name of next file. Will not exceed
 *             MAX_FILENAME_LENGTH.
 *  filestat - buffer in which to store metadata about the next file.
 * Returns 0 on success. Sets errno.
 * Returns -1 and sets errno to ENOENT if no more files avilable. */
static inline int SD_dirnext(
    DIR_STATE_STRUCT *state, char *filename, struct stat *filestat);
/* Closes a directory iterator.
 * Returns 0 on success. Sets errno. */
static inline int SD_dirclose(DIR_STATE_STRUCT *state);

/******************************************************************************/

extern PARTITION sd_partition;

static inline int SD_open(
        FILE_STRUCT *fileStruct, const char *path, int flags) {
    return FAT_open(fileStruct, &sd_partition, path, flags);
}
static inline int SD_close(int fd) {
    return FAT_close(fd);
}
static inline ssize_t SD_write(int fd, const char *ptr, size_t len) {
    return FAT_write(fd, ptr, len);
}
static inline ssize_t SD_read(int fd, char *ptr, size_t len) {
    return FAT_read(fd, ptr, len);
}
static inline off_t SD_seek(int fd, off_t pos, int dir) {
    return FAT_seek(fd, pos, dir);
}

static inline int SD_ftruncate(int fd, off_t len) {
    return FAT_ftruncate(fd, len);
}
static inline int SD_fsync(int fd) {
    return FAT_fsync(fd);
}
static inline int SD_fstat(int fd, struct stat *st) {
    return FAT_fstat(fd, st);
}

static inline FILE_ATTR SD_getAttr(const char *file) {
    return FAT_getAttr(&sd_partition, file);
}
static inline int SD_setAttr(const char *file, FILE_ATTR attr) {
    return FAT_setAttr(&sd_partition, file, attr);
}

static inline int SD_stat(const char *path, struct stat *st) {
    return FAT_stat(&sd_partition, path, st);
}
static inline int SD_statvfs(const char *path, struct statvfs *buf) {
    return FAT_statvfs(&sd_partition, path, buf);
}

static inline int SD_unlink(const char *name) {
    return FAT_unlink(&sd_partition, name);
}
static inline int SD_rename(const char *oldName, const char *newName) {
    return FAT_rename(&sd_partition, oldName, newName);
}

static inline int SD_chdir(const char *name) {
    return FAT_chdir(&sd_partition, name);
}

static inline int SD_mkdir(const char *path) {
    return FAT_mkdir(&sd_partition, path);
}

static inline DIR_STATE_STRUCT* SD_diropen(
        DIR_STATE_STRUCT *state, const char *path) {
    return FAT_diropen(state, &sd_partition, path);
}
static inline int SD_dirreset(DIR_STATE_STRUCT *state) {
    return FAT_dirreset(state);
}
static inline int SD_dirnext(
        DIR_STATE_STRUCT *state, char *filename, struct stat *filestat) {
    return FAT_dirnext(state, filename, filestat);
}
static inline int SD_dirclose(DIR_STATE_STRUCT *state) {
    return FAT_dirclose(state);
}

#endif /* _IO_FAT_SD_H_ */
