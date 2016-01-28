#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define ERROR_OUTPUT stderr

extern int yylineno, yycolno;
extern char * yyfilename;

static int errc = 0;

static void error_(const char *format, va_list ap, const char *message)

{
	errc ++;
	fprintf(ERROR_OUTPUT, "%s:%d:%d: ", yyfilename, yylineno, yycolno);
	fprintf(ERROR_OUTPUT, "%s: \e[96m", message);
	vfprintf(ERROR_OUTPUT, format, ap);
	fprintf(ERROR_OUTPUT, "\e[39m");
}

int error_count(void)
{
	return errc;
}

void warning(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	error_(format, ap, " \e[93mwarning");
}

void error(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	error_(format, ap, "\e[91merror");
}

void fatal_error(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	error_(format, ap,"\e[91mfatal error");
	
	exit(EXIT_FAILURE);
}
