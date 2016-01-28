#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.hpp"
#include "type.hpp"
#include "symbol_table.hpp"
#include "error.hpp"

struct symbol *symbol_new(const char *name, const struct type *t)
{
	struct symbol *sy = malloc(sizeof*sy);
	sy->name = name;
	sy->type = t;
	
	sy->variable = (struct symbol_variable) { 0};
	
	return sy;
}

void symbol_print(const struct symbol *sy)
{
	printf("SYMBOL: ");
	printf("%s\n", sy->name);
	type_print(sy->type, -1);
	puts("\n");
}

struct symbol * symbol_check(const char *name)
{
	struct symbol *sy = NULL;
	if ( !st_search(name, &sy) ) {
	error("undefined reference to %s\n", name);
	sy = symbol_new(name, type_generic);
	}
	return sy;
}
