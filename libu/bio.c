#include <stdio.h>

#include "uimpl.h"

void putint(int x)
{
    printf("%d", x);
}

void putfloat(float x)
{
	printf("%f", x);
}

void putaddr(struct uuarray*a)
{
	printf("%p", a);
}

void putstr(char *str)
{
    fputs(str, stdout);
}

void putendl(void)
{
    printf("\n");
}
