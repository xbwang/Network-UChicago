/* error.c - error */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * error - print an error message and exit
 *------------------------------------------------------------------------
 */

/*VAR ARGS1*/
int
error(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}
