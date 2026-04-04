/*
 * LWSDK Library Source File
 * Copyright 1995  NewTek, Inc.
 * Modified for GCC cross-compilation
 */
#include <splug.h>

#if defined(_AMIGA) || defined(__AMIGA__) || defined(__amigaos__)
#include <proto/exec.h>
#include <proto/dos.h>

struct ExecBase         *SysBase;
struct DosLibrary       *DOSBase;

/*
 * GCC's libnix math stubs (___adddf3, ___muldf3, sqrt, etc.) call through
 * these library base pointers. In a normal program, the C startup opens
 * them. Since plugins use -nostartfiles, we must open them here.
 * They are declared as weak references so plugins that don't use math
 * won't pull them in or fail if the symbols aren't present.
 */
#ifdef __GNUC__
extern struct Library *MathIeeeDoubBasBase   __attribute__((weak));
extern struct Library *MathIeeeDoubTransBase __attribute__((weak));
#endif

#ifdef __SASC
       extern int  __stdargs    __fpinit (void);
       extern void __stdargs    __fpterm (void);
#endif

	XCALL_(void *)
_Startup (void)
{
	void                    *val;

	XCALL_INIT;

	SysBase = *((struct ExecBase **)4);
	DOSBase = (struct DosLibrary *) OpenLibrary ((UBYTE*) "dos.library", 37L);
	if (!DOSBase)
		return NULL;

#ifdef __GNUC__
	if (&MathIeeeDoubBasBase)
		MathIeeeDoubBasBase = OpenLibrary (
			(UBYTE*) "mathieeedoubbas.library", 0L);
	if (&MathIeeeDoubTransBase)
		MathIeeeDoubTransBase = OpenLibrary (
			(UBYTE*) "mathieeedoubtrans.library", 0L);
#endif

	#ifdef __SASC
	       if (__fpinit ())
		       goto die;
	#endif

	val = Startup ();
	if (val)
		return val;

#ifdef __GNUC__
	if (&MathIeeeDoubTransBase && MathIeeeDoubTransBase)
		CloseLibrary (MathIeeeDoubTransBase);
	if (&MathIeeeDoubBasBase && MathIeeeDoubBasBase)
		CloseLibrary (MathIeeeDoubBasBase);
#endif
	#ifdef __SASC
	       __fpterm ();
	#endif
	CloseLibrary ((struct Library *) DOSBase);
	return NULL;
}

	XCALL_(void)
_Shutdown (
	void                    *serverData)
{
	XCALL_INIT;

	Shutdown (serverData);

#ifdef __GNUC__
	if (&MathIeeeDoubTransBase && MathIeeeDoubTransBase)
		CloseLibrary (MathIeeeDoubTransBase);
	if (&MathIeeeDoubBasBase && MathIeeeDoubBasBase)
		CloseLibrary (MathIeeeDoubBasBase);
#endif
	#ifdef __SASC
	       __fpterm ();
	#endif
	CloseLibrary ((struct Library *) DOSBase);
}
#endif
