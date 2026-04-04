// LWEnvlp.h   -- Global Plugin Handler for Lightwave3D Animation Envelopes
// 1/4/97 Copyright 1996,1997  Newtek Inc.
// 2/11/97 - API version 2: clients now pass ehInterfaceData into interface()
// 10/9/97 - API version 3:	key manipulation functions available, 
//	all 1 channel envelopes multiplied by 100 on loading, and divided on saving, for compatibility
// 12/10/97  - Change interface to Interface, which does not seem to ne a reserved word yet.
// 3/25/98 - added stream load/save
#ifndef	PANEL_SERVICES_NAME
#include <lwpanel.h>
#endif

#define ENVELOPE_ACCESS_NAME  "EnvelopeHandler"
#define ENVELOPE_ACCESS_API_VERSION  4

#define MAX_CHANNELS	16
typedef double		 ChanVec[MAX_CHANNELS];
typedef struct st_KeyFrame {
	void		*prev,*next;  // (Keyframe *)
	ChanVec		cv;
	double		tens, cont, bias;
	int		 	linear;
	int		 	step; 
} Keyframe;

#define MOTF_STOP		1  // Motion.flags endbehavior		
#define MOTF_REPEAT		2  // Motion.flags endbehavior

typedef struct st_Motion {
	Keyframe	*keylist;
	int			 keys, steps, channels, flags;
} Motion;

typedef Motion Envelope;   // 1 channel motion, generally

typedef struct st_channelDescription
{
	char *chname;		 // Label for popup
	float min, max;		 // acceptable value ranges
} channelDescription;

typedef struct st_ehInterfaceData {	
	LWPanelFuncs 		*pfunc;
	const char 			*title;
	channelDescription	chDesc[MAX_CHANNELS];
} ehInterfaceData;

typedef struct st_EnvelopeHandler {
	int 	api_version;
	Envelope *(*create)(LWError *);
	void (*destroy)(Envelope *);
	LWError (*copy)(Envelope *from,Envelope *to);
	LWError (*load)(Envelope *env,char *filename);
	LWError (*save)(Envelope *env, char *filename);

	void (*evaluate)(Envelope *, LWTime, ChanVec);
	int	(*Interface)(Envelope *, ehInterfaceData *dat);		 // return 1 on OK, 0 on cancel(undo), -1 on remove	(client must destroy() Envelope and copy
// API V3
	LWError (*LWLoad)(Envelope *env, const LWLoadState *);
	LWError (*LWSave)(Envelope *env, const LWSaveState *);
	Keyframe *(*createKey)(int step,ChanVec cv,int chans); // allocate and fill key 
	void (*putKey)(Envelope *env,Keyframe *key);	  // insert key in envelope, replacing any existing key at the same step
	Keyframe *(*getKey)(Envelope *env, int step);	  // return key in envelope at step, or NULL is no key there
	void (*destroyKey)(Keyframe *key);				  // remove key from envelope and free 
// API V4
	LWError (*loadStream)(Envelope *env, FILE *fp);		// load env from open stream
	LWError (*saveStream)(Envelope *env, FILE *fp);		// save env from open stream
} EnvelopeHandler;

