#include <stdio.h>

#include "uimpl.h"

void putint(int x)
{
    printf("%d\n", x);
}

void putfloat(float x)
{
	printf("%f\n", x);
}

void putaddr(struct uuarray*a)
{
	printf("%p\n", a);
}
