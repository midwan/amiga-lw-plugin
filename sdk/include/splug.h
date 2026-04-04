/*
 * LWSDK Header File
 * Copyright 1995  NewTek, Inc.
 */
#ifndef TOOLS_SPLUG_H
#define TOOLS_SPLUG_H

#include <plug.h>

typedef struct st_ServerRecord {
	const char      *_class;
	const char      *name;
	ActivateFunc    *activate;
} ServerRecord;
extern         void *
Startup (void);
extern         void
Shutdown (
	void                    *serverData);

#endif
