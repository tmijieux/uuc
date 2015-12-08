
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define HEURISTIC_SIZE 11

char *strdup(const char *s)
{
	char * d = malloc(strlen(s+1));
	strcpy(d, s);
	return d;
}


int asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;

	char * buf = malloc ( HEURISTIC_SIZE );
	
	va_start(ap, fmt);
	int n = vsnprintf(buf, HEURISTIC_SIZE, fmt, ap);
	if ( n >= HEURISTIC_SIZE )
	{
	    buf = realloc( buf, n+1 );
	    va_start(ap, fmt); // important !!
	    vsnprintf(buf, n+1, fmt, ap);
	}

	*strp = buf;
	return n;
}
