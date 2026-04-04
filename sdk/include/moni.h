/*
 * LWSDK Header File
 * Copyright 1995  NewTek, Inc.
 */
#ifndef TOOLS_MONI_H
#define TOOLS_MONI_H

typedef struct st_Monitor {
	void             *data;
	void            (*init) (void *, unsigned int);
	int             (*step) (void *, unsigned int);
	void            (*done) (void *);
} Monitor;
#define MON_INIT(mon,count)     if (mon) (*mon->init) (mon->data, count)
#define MON_INCR(mon,d)         (mon ? (*mon->step) (mon->data, d) : 0)
#define MON_STEP(mon)           MON_INCR (mon, 1)
#define MON_DONE(mon)           if (mon) (*mon->done) (mon->data)

#endif
