/*
 * LWSDK Header File
 * Copyright 1995  NewTek, Inc.
 */
#ifndef TOOLS_IMAGE_H
#define TOOLS_IMAGE_H

#include <moni.h>

typedef unsigned char            ImageValue;
typedef union un_ImageProtocol  *ImageProtocolID;
typedef struct st_ImLoaderLocal {
	void             *priv_data;
	int               result;
	const char       *filename;
	Monitor          *monitor;
	ImageProtocolID (*begin) (void *, int type);
	void            (*done) (void *, ImageProtocolID);
} ImLoaderLocal;
typedef struct st_ImSaverLocal {
	void            *priv_data;
	int              result;
	int              type;
	const char      *filename;
	Monitor         *monitor;
	int            (*sendData) (void *, ImageProtocolID, int);
} ImSaverLocal;
typedef struct st_ColorProtocol {
	int              type;
	void            *priv_data;
	void           (*setSize) (void *, int, int, int);
	int            (*sendLine) (void *, int, const ImageValue *,
				    const ImageValue *);
	int            (*done) (void *, int);
} ColorProtocol;
typedef struct st_IndexProtocol {
	int              type;
	void            *priv_data;
	void           (*setSize) (void *, int, int, int);
	void           (*numColors) (void *, int);
	void           (*setMap) (void *, int, const ImageValue[3]);
	int            (*sendLine) (void *, int, const ImageValue *,
				    const ImageValue *);
	int            (*done) (void *, int);
} IndexProtocol;
typedef union un_ImageProtocol {
	int              type;
	ColorProtocol    color;
	IndexProtocol    index;
} ImageProtocol;
#define IMG_RGB24       0
#define IMG_GREY8       1
#define IMG_INDEX8      2
#define IPSTAT_OK                0
#define IPSTAT_NOREC             1
#define IPSTAT_BADFILE           2
#define IPSTAT_ABORT             3
#define IPSTAT_FAILED           99
#define IMGF_ALPHA               1
#define IMGF_REVERSE             2
#define IP_SETSIZE(p,w,h,f)     (*(p)->setSize) ((p)->priv_data,w,h,f)
#define IP_NUMCOLORS(p,n)       (*(p)->numColors) ((p)->priv_data,n)
#define IP_SETMAP(p,i,val)      (*(p)->setMap) ((p)->priv_data,i,val)
#define IP_SENDLINE(p,ln,d,a)   (*(p)->sendLine) ((p)->priv_data,ln,d,a)
#define IP_DONE(p,err)          (*(p)->done) ((p)->priv_data,err)

#endif
