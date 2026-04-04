/*
 * LWSDK Library Source File
 * Copyright 1995,1997,1998  NewTek, Inc.
 *
 * functions for Layout plugin load/save i/o
 * by Arnie Cachelin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pluginio.h"

void writeInt(const LWSaveState	*sState, int dval)
{
	if (sState->ioMode == LWIO_OBJECT) {
		int		 val = dval;
		BSWAP_L(val);
		(*sState->write) (sState->writeData, (char *) &val, 4);
	} else {
		char		 buf[50];
		sprintf (buf, " %d", dval);
		(*sState->write) (sState->writeData, buf, strlen (buf) + 1);
	}
}

int readInt(const LWLoadState	*lState)
{
	int		 val=0;
	if (lState->ioMode == LWIO_OBJECT) {
		(*lState->read) (lState->readData, (char *) &val, 4);
		BSWAP_L(val);
		return val;
	} else {
		char		 buf[50];
		(*lState->read) (lState->readData, buf, 50);
		sscanf (buf, "%d", &val);
		return val;
	}
}

void writeFloat(const LWSaveState	*sState, float dval)
{
	if (sState->ioMode == LWIO_OBJECT) {
		float		 val = dval;
		BSWAP_L(val);
		(*sState->write) (sState->writeData, (char *) &val, 4);
	} else {
		char		 buf[50];
		sprintf (buf, " %f", dval);
		(*sState->write) (sState->writeData, buf, strlen (buf) + 1);
	}
}

float readFloat(const LWLoadState	*lState)
{
	if (lState->ioMode == LWIO_OBJECT) {
		float		 val;
		(*lState->read) (lState->readData, (char *) &val, 4);
		BSWAP_L(val);
		return val;
	} else {
		char		 buf[50];
		double		 val;
		(*lState->read) (lState->readData, buf, 50);
		sscanf (buf, "%f", &val);
		return (float) val;
	}
}

// 2-byte length, possibly followed by even length	including NULL term.
void writeString(const LWSaveState	*sState, char *str)
{
	if (sState->ioMode == LWIO_OBJECT) 
	{
		WORD		 len;
		if(!str || !*str)
		{
			len = 0;
			(*sState->write) (sState->writeData, (char *) &len, 2);
			return ;
		}
		len = strlen(str);	  //  >0
		len++;	   // null
		BSWAP_W(len);
		(*sState->write) (sState->writeData, (char *) &len, 2);
		BSWAP_W(len);
		if(len)
		{
			(*sState->write) (sState->writeData, str, len);
			if(len&1) // odd size
			{
				len = 0;
				(*sState->write) (sState->writeData, (char *) &len, 1);
			}
		}
	} 
	else {
		(*sState->write) (sState->writeData, str, strlen(str) + 1);
	}
}

int readString(const LWLoadState	*lState, char *str, int maxStr)
{
	if (lState->ioMode == LWIO_OBJECT) 
	{
		WORD		 len=0;
		if(!str || !maxStr)
			return 0;
		(*lState->read) (lState->readData, (char *) &len, 2);
		BSWAP_W(len);
		if(len>maxStr) 
		{
			int duh;
			(*lState->read) (lState->readData,  str, maxStr);
			len -= maxStr;
			while(len>4)
			{
				(*lState->read) (lState->readData,  (char *)&duh, 4);
				len -= 4;
			}
			while(len)
			{
				(*lState->read) (lState->readData,  (char *)&duh, 1);
				len--;
			}
			return maxStr;
		}
		(*lState->read) (lState->readData,  str, len);
		if(len&1) // odd size
			(*lState->read) (lState->readData,  (char *) &len, 1);
	} 
	else 
		return	(*lState->read) (lState->readData,  str, maxStr);
}

