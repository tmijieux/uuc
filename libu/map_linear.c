#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uimpl.h"

void map(struct mapcont *mc)
{
    if ( mc->mtype == MT1 )
    {
        uint32_t (*fun4)(uint32_t) = mc->fun;

        for (int i = 0; i < mc->param->size; ++i)
        {
            *(((uint32_t*)(mc->ret->buffer))+i) =
                fun4(*(((uint32_t*)(mc->param->buffer))+i));
        }
    }
    else if ( mc->mtype == MT2 )
    {
        uint32_t (*fun48)(uint64_t) = mc->fun;

        for (int i = 0; i < mc->param->size; ++i)
        {
            void * addr = *(((void**)(mc->param->buffer))+i);
            *(((uint32_t*)mc->ret->buffer)+i) =
                fun48((uint64_t) addr);
        }
    }
    else if ( mc->mtype == MT3 )
    {
        uint64_t (*fun84)(uint32_t) = mc->fun;
				
        for (int i = 0; i < mc->param->size; ++i)
        {
            *(((uint64_t*)mc->ret->buffer)+i) =
                fun84(*(((uint32_t*)(mc->param->buffer))+i));
        }
    }
    else if ( mc->mtype == MT4 )
    {
        uint64_t (*fun8)(uint64_t) = mc->fun;

        for (int i = 0; i < mc->param->size; ++i)
        {
            void * addr = *(((void**)(mc->param->buffer))+i);
            *(((uint64_t*)mc->ret->buffer)+i) =
                fun8((uint64_t) addr);
        }
    }
}
