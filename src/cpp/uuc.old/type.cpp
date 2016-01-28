#include <stdio.h>
#include <cstring>
#include "type.hpp"
#include "list.hpp"
#include "symbol.hpp"

struct type *type_new(enum enum_type t);

const struct type* type_undef;
const struct type* type_generic;
const struct type* type_int; 
const struct type* type_float;
const struct type* type_void; 

__attribute__((constructor))
static void construct_type(void)
{
    type_undef = type_new(TYPE_UNDEF);
    type_generic = type_new(TYPE_GENERIC);
    type_int = type_new(TYPE_INT);
    type_float = type_new(TYPE_FLOAT);
    type_void = type_new(TYPE_VOID);
}

/**
 *  This variable is used to remember the last type_name that was read
 *  to allow the program to know which type_name it should apply when
 *  it encounters a declarator
 */
const struct type* last_type_name = NULL;


/********************************************/

struct type *type_new(enum enum_type et)
{
    struct type *t = malloc(sizeof*t);
    t->type = et;
    switch (et) {
    case TYPE_INT:
	t->size = SIZE_INT;
	break;
    case TYPE_FLOAT:
	t->size = SIZE_FLOAT;
	break;
	
    case TYPE_ARRAY:	
    case TYPE_FUNCTION:
	t->size = SIZE_ADDR;
	break;
	
    default:
	t->size = 0;
	break;
    }
    return t;
}

/**************************************************/


const struct type *
type_new_array_type(const struct type *values, size_t array_size)
{
    struct type *ty = type_new(TYPE_ARRAY);
    ty->array_type.values = values;
    ty->array_type.array_size = array_size;
    return ty;
}

const struct type *
type_new_function_type(const struct type *return_value, struct list *args)
{
    struct type *ty = type_new(TYPE_FUNCTION);
    ty->function_type.return_value = return_value;
    ty->function_type.args = args;
    ty->function_type.argc = list_size(args);
    return ty;
}

static void print_tab(int n)
{
    ++n;
    while (n--)
	printf("\t");
}

void  type_print_line(const struct type *t) 
{
    type_print(t, -1);
}

void type_print(const struct type *t, int level)
{
    switch ( t->type )
    {
    case TYPE_UNDEF:
	printf("undef");
	break;
    case TYPE_GENERIC:
	printf("generic");
	break;
	
    case TYPE_VOID:
	printf("void");
	break;
	
    case TYPE_INT:
	printf("int");
	break;

    case TYPE_FLOAT:
	printf("float");
	break;

    case TYPE_ARRAY:
	printf("array<");
	type_print(t->array_type.values, level+1);

	printf(">");
	break;

    case TYPE_FUNCTION:
	printf("function (");
	{
	    struct list *l = t->function_type.args;
	    int s = list_size(l);
	    for (int i = 1; i <= s; ++i) {
		if (i > 1)
		    printf(", ");
		if ( level >= 0 ) printf("\n");
		print_tab(level);
		struct symbol *v = list_get(l, i);
		type_print(v->type, level>=0?level+1:level);
	    }
		
	    if ( level >= 0 && s > 0 ) {
		puts("");
		print_tab(level);
	    } 
	    printf(") --> ");
	    type_print(t->function_type.return_value, level+1);
	}
	break;
	
    default:
	break;
    }
}


int type_equal(const struct type *t1, const struct type *t2) 
{
    if ( t1->type != t2->type )
    {
	if ( t1->type == TYPE_GENERIC || t2->type == TYPE_GENERIC )
	    return 1;
	return 0;
    }
	
    if ( t1->type == TYPE_FUNCTION )
    {
	if ( !type_equal(t1->function_type.return_value,
			 t2->function_type.return_value) )
	    return 0;
	struct list *l1 = t1->function_type.args;
	struct list *l2 = t2->function_type.args;
	unsigned int s;
	if ( (s = list_size(l1)) != list_size(l2) )
	    return 0;
	for (unsigned int i = 1; i <= s; ++i) {
	    if (!type_equal(list_get(l1, i), list_get(l2, i)))
		return 0;
	}
    }
	
    if ( t1->type == TYPE_ARRAY )
    {
	if ( !type_equal(t1->array_type.values, t2->array_type.values) )
	    return 0;
    }

    return 1;
}
