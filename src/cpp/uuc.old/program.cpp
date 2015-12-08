#include <stdio.h>
#include "list.hpp"
#include "program.hpp"
#include "error.hpp"

static unsigned int unique = 0;

struct list *fun_code = NULL;

__attribute__((constructor))
static void program_init(void)
{
	fun_code = list_new(0);
}

void prgm_print_code(void)
{
	if ( error_count() == 0 )
	{
	int s = list_size(fun_code);
	puts("declare i8* @GC_malloc(i64 %s)");
	for (int i = 1; i <= s; ++i)
		puts((const char*) list_get(fun_code, i));
	}
}

unsigned int prgm_get_unique_id(void)
{
	return unique ++;
}
