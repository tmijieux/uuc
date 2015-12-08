#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.hpp"
#include "hash_table.hpp"
#include "list.hpp"
#include "error.hpp"
#include "program.hpp"
#include "llvm.hpp"

struct symbol_table {
	int level;
	struct hash_table *ht;
	struct symbol_table *next;

	int64_t size;
};

static int init__ = 0, exit__ = 0;

static struct symbol_table *st = NULL;

/**
 *  This is a temporary variable used to save
 *  parameter of function definition to insert them
 *  once the function compound statement really starts
 */
static struct list *function_parameters = NULL;

void st_set_parameters(struct list *l)
{
	if (st->level == 0) // this has no meaning inside a function
	{
	function_parameters = l;
	}
}

/**
 * initialize the symbol_table stack module
 * no action within the module is to be considered valid
 * before this function returns
 *
 */

void st_init(void)
{
	if ( !init__ ) {
	st = (struct symbol_table*) malloc(sizeof*st);
	st->ht = ht_create(100, NULL);
	st->next = NULL;
	st->level = 0;
	
	init__ = 1;
	}
}

/**
 * add the symbol 'sy' at the top of the symbol_table stack
 * if the symbol already exist **at the top** of the stack
 * no operation is performed and the function returns 0
 * otherwise the symbol is added, and the function returns 1
 *
 * N.B.: the symbol is added even if a symbol with the same name
 * exists somewhere **strictly lower than the top** in the stack
 */

int st_add(struct symbol *sy)
{
	if ( ht_has_entry(st->ht, sy->name) )
	return 0;
	if (st->level >= 1) {
	char *code;
	asprintf(&code,
		 "%%%s.stack = alloca %s",
		 sy->name,
		 llvm_type(sy->type));

	
	if (sy->variable.is_parameter) {
		const char *type = llvm_type(sy->type);
		asprintf(&code, "%s\n"
			 "store %s %%%s, %s* %%%s.stack",
			 code,  type, sy->name, type, sy->name);
	} else {
		if (sy->type->type == TYPE_ARRAY &&
		sy->type->array_type.array_size != 0)
		{
		const char *type = llvm_type(sy->type);
		// alloc  space for the array:
		int d1 = prgm_get_unique_id();
		int d2 = prgm_get_unique_id();

		// TODO: recursive alloc if multiple dimension array
		asprintf(&code,
			 "%s\n"
			 "%%t%u = call i8* @GC_malloc(i64 %zu)\n"
			 "%%t%u = bitcast i8* %%t%u to %s\n"
			 "store %s %%t%u, %s* %%%s.stack",
			 code,
			 d1, sy->type->array_type.array_size,
			 d2, d1, type,
			 type, d2, type, sy->name);
		}
	}
	sy->variable.alloc_code = code;
	}
	
	ht_add_entry(st->ht, sy->name, sy);
	return 1;
}

/**
 * search for symbol 'name'
 * if found, returns 1 and the symbol in *sy_ret
 * returns  0 otherwise
 */
int st_search(const char *name, struct symbol **sy_ret)
{
	struct symbol_table *syt = st;

	while (syt != NULL)
	{
	if ( ht_get_entry(syt->ht, name, sy_ret) == 0 )
		return 1;
	syt = syt->next;
	}
	return 0;
}

/**
 * pop a symbol_table from the symbol table's stack
 * this is used to end the scope of local variables in a compound statement
 */
void st_pop(void)
{
	if (st->level > 0) {
	ht_free(st->ht);
	struct symbol_table *tmp = st->next;
	free(st);
	st = tmp;
	}
}

/**
 * push a symbol_table to the symbol table's stack
 * this is used to create a new scope for  local variables in a compound statement
 */
void st_push(void)
{
	struct symbol_table *tmp = (struct symbol_table*) malloc(sizeof*tmp);
	tmp->ht = ht_create(100, NULL);
	tmp->next = st;
	tmp->level = st->level + 1;
	st = tmp;

	if (st->level == 1) // if we just entered inside a function
	{
	if (function_parameters == NULL)
		return; // no arguments to the function
	
	int s = list_size(function_parameters);
	for (int i = 1; i <= s; ++i)
	{ // push the function parameters to the declared symbols
		struct symbol *tmp = (struct symbol*) list_get(function_parameters, i);
		if (!st_add(tmp))
		error("symbol multiple definition : %s \n", tmp->name);
	}
	function_parameters = NULL; // ... and reset the list
	}
}

/**
 * exit the symbol_table stack module
 * no action within this module is to be considered valid
 * after this function was called
 *
 */
void st_exit(void)
{
	if ( !exit__ ) {
	st = NULL;
	init__ = 1;
	exit__ = 1;
	}
}

/**
 * return the level of the symbol table
 * 0 is for out of any function
 * 1 if for a function
 * >= 2 is for an compound instruction (block) within a function
 */

int st_level(void)
{
	return st->level;
}



