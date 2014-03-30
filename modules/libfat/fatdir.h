/*
 fatdir.h
 
 Functions used by the newlib disc stubs to interface with 
 this library

 Edited 2014 by Alex Chadwick for inclusion in bslug
 Copyright (c) 2006 Michael "Chishm" Chisholm
	
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


#ifndef _FATDIR_H
#define _FATDIR_H

#include <sys/reent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "common.h"
#include "directory.h"

typedef struct {
	PARTITION* partition;
	DIR_ENTRY  currentEntry;
	uint32_t   startCluster;
	bool       inUse;
	bool       validEntry;
} DIR_STATE_STRUCT;

extern int FAT_stat_r (struct _reent *r, PARTITION *partition, const char *path, struct stat *st);

extern int FAT_link_r (struct _reent *r, PARTITION *partition, const char *existing, const char *newLink);

extern int FAT_unlink_r (struct _reent *r, PARTITION *partition, const char *name);

extern int FAT_chdir_r (struct _reent *r, PARTITION *partition, const char *name);

extern int FAT_rename_r (struct _reent *r, PARTITION *partition, const char *oldName, const char *newName);

extern int FAT_mkdir_r (struct _reent *r, PARTITION *partition, const char *path, int mode);

extern int FAT_statvfs_r (struct _reent *r, PARTITION *partition, const char *path, struct statvfs *buf);

/*
Directory iterator functions
*/
extern DIR_STATE_STRUCT* FAT_diropen_r(struct _reent *r, DIR_STATE_STRUCT *state, PARTITION *partition, const char *path);
extern int FAT_dirreset_r (struct _reent *r, DIR_STATE_STRUCT *state);
extern int FAT_dirnext_r (struct _reent *r, DIR_STATE_STRUCT *state, char *filename, struct stat *filestat);
extern int FAT_dirclose_r (struct _reent *r, DIR_STATE_STRUCT *state);


#endif // _FATDIR_H
