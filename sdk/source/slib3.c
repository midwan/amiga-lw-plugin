/*
 * LWSDK Library Source File
 * Copyright 1995  NewTek, Inc.
 */
#include <splug.h>
extern char             ServerClass[];
extern char             ServerName[];

extern         XCALL_(int)
Activate (long             version,
GlobalFunc      *global,
void            *local,
void            *serverData);

	ServerRecord
ServerDesc[] = {
	{ ServerClass, ServerName, Activate },
	{ 0 }
};
