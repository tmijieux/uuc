#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "uimpl.h"


struct thdata {
	struct mapcont mc;
	int _0, len;
};

static void die(const char *str)
{
	fputs(str, stderr);
	exit(EXIT_FAILURE);
}

static void *thjob(void *o)
{
	struct thdata td = *(struct thdata*) o;
	struct mapcont *mc = &td.mc;

	if ( mc->mtype == MT1 )
	{
		uint32_t (*fun4)(uint32_t) = mc->fun;

		for (int i = 0; i < td.len; ++i)
		{
			*(((uint32_t*)(mc->ret->buffer))+i+td._0) =
				fun4(*(((uint32_t*)(mc->param->buffer))+i+td._0));
		}
	}
	else if ( mc->mtype == MT2 )
	{
		uint32_t (*fun48)(uint64_t) = mc->fun;

		for (int i = 0; i < td.len; ++i)
		{
			void * addr = *(((void**)(mc->param->buffer))+i+td._0);
			*(((uint32_t*)mc->ret->buffer)+i+td._0) =
				fun48((uint64_t) addr);
		}
	}
	else if ( mc->mtype == MT3 )
	{
		uint64_t (*fun84)(uint32_t) = mc->fun;
				
		for (int i = 0; i < td.len; ++i)
		{
			*(((uint64_t*)mc->ret->buffer)+i+td._0) =
				fun84(*(((uint32_t*)(mc->param->buffer))+i+td._0));
		}
	}
	else if ( mc->mtype == MT4 )
	{
		uint64_t (*fun8)(uint64_t) = mc->fun;

		for (int i = 0; i < td.len; ++i)
		{
			void * addr = *(((void**)(mc->param->buffer))+i+td._0);
			*(((uint64_t*)mc->ret->buffer)+i+td._0) =
				fun8((uint64_t) addr);
		}
	}

	pthread_exit(NULL);
}

void map(struct mapcont *mc)
{
	pthread_t t[2];

	struct thdata td[2];
	pthread_attr_t attr;

	pthread_attr_init(&attr);


	td[1].mc = *mc;
	td[1]._0 = mc->param->size/2;
	td[1].len = mc->param->size - mc->param->size/2;
	
	if ( pthread_create(&t[1], &attr, thjob, &td[1]) != 0 )
		die("pthread_create failed\n");
	
	td[0].mc = *mc;
	td[0]._0 = 0;
	td[0].len = mc->param->size / 2;
	
	if ( pthread_create(&t[0], &attr, thjob, &td[0]) != 0)
		die("pthread_create failed\n");
	
	pthread_join(t[0], NULL);
	pthread_join(t[1], NULL);

	pthread_attr_destroy(&attr);

}
