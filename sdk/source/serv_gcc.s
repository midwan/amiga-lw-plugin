/*
 * GCC server startup module for LightWave plugins
 * Based on the SAS/C serv_s.a from the LW SDK
 *
 * The plugin binary format starts with:
 *   moveq #0,d0 / rts    (so LoadSeg returns 0 if run from CLI)
 *   magic: $04121994
 *   flags: $2
 *   version: $100
 *   pointer to _Startup
 *   pointer to _Shutdown
 *   pointer to ServerDesc
 */

	.text
	.even
	.globl _start
_start:
	moveq	#0,%d0
	rts

	.long	0x04121994
	.long	0x2
	.long	0x100
	.long	__Startup
	.long	__Shutdown
	.long	_ServerDesc

/* Dummy __XCEXIT for compatibility */
	.globl	___XCEXIT
___XCEXIT:
	.long	0
	.long	0
	.long	0
