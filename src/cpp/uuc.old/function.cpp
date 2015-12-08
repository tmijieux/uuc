#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>

#include "function.hpp"
#include "symbol.hpp"
#include "hash_table.hpp"
#include "llvm.hpp"

struct hash_table *ft;

// TODO Replace symbol by struct
// with code and all things that goes well

void fun_init(void)
{
	ft = ht_create(100, NULL);
}

void fun_exit(void)
{
	ht_free(ft);
	ft = NULL;
}

int fun_defined(struct symbol *symb)
{
	return ht_has_entry(ft, symb->name);
}

void fun_define(struct symbol *symb)
{
	if ( !fun_defined(symb) )
	ht_add_entry(ft, symb->name, symb);
}

struct function * fun_new(struct symbol *sy, const char *code)
{
	struct function *fun = (struct function*) malloc(sizeof*fun);
	fun->symb = sy;
	
	char *funcode = NULL;
	asprintf(&funcode,
		 "define %s @%s(%s) { \n"
		 "start:\n"
		 "%s\n"
		 "}\n",
		 llvm_type(sy->type->function_type.return_value),
		 sy->name,
		 llvm_arglist(sy->type->function_type.args),
		 code);
	fun->code = funcode;
	return fun;
}
