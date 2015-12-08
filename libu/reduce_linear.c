#include <stdlib.h>
#include <stdint.h>

#include "uimpl.h"

void reduce(struct redcont *mc)
{
	if ( mc->rtype == MT1 )
	{
		uint32_t (*fun4)(uint32_t, uint32_t) = mc->fun;
		uint32_t res = 	*((uint32_t*)mc->param->buffer);
		for (int i = 1; i < mc->param->size; ++i)
			res = fun4(res, *(((uint32_t*)mc->param->buffer)+i));
		*((uint32_t*)(mc->ret)) = res;
	}
	else if ( mc->rtype == MT2 )
	{
		uint64_t (*fun8)(uint64_t, uint64_t) = mc->fun;
		uint64_t res = 	*((uint64_t*)mc->param->buffer);
		for (int i = 1; i < mc->param->size; ++i)
			res = fun8(res, *(((uint64_t*)mc->param->buffer)+i));
		*((uint64_t*)(mc->ret)) = res;
	}
}
