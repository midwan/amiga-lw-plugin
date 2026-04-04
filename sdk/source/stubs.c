/*
 * Stub functions for plugin environment.
 * Plugins are loaded by LightWave, never run standalone,
 * so exit() is never called but libnix references it.
 */

void exit(int code)
{
	(void)code;
	while (1)
		;
}
