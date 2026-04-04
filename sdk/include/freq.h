/*
 * LWSDK Header File
 * Copyright 1995  NewTek, Inc.
 */
#ifndef TOOLS_FREQ_H
#define TOOLS_FREQ_H

#include <plug.h>

typedef struct {
	int              reqType, result;
	const char      *title;
	const char      *fileType;
	char            *path;
	char            *baseName;
	char            *fullName;
	int              bufLen;
} FileReq_Local;

#define FREQ_GENERIC     0
#define FREQ_LOAD        1
#define FREQ_SAVE        2
typedef const char      *FileTypeFunc (const char *);
extern         int
FileReq_SysLocal (
	long                     version,
	GlobalFunc              *global,
	FileReq_Local           *local,
	void                    *dummy);

#endif
