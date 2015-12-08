#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <assert.h>
#include <cstring>

#include "llvm.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "list.hpp"
#include "expression.hpp"


llvm::LLVMContext &LLVM::context = llvm::getGlobalContext();
llvm::IRBuilder<> LLVM::Builder(context);
llvm::Module *LLVM::module = new llvm::Module("program", context);


const char *llvm_type(const struct type *t)
{
    char *lt = "";
	
    switch (t->type)
    {
    case TYPE_ARRAY:
	//		{i32, [ 100 x type ] }
	asprintf(&lt, "[ %zu x %s ]*",
		 t->array_type.array_size,
		 llvm_type(t->array_type.values));
	break;
    case TYPE_FUNCTION:
	asprintf(&lt, "%s (%s) *",
		 llvm_type(t->function_type.return_value),
		 llvm_arglist_type(t->function_type.args));
	break;

    case TYPE_FLOAT:
	lt = (char*) "float";
	break;
    case TYPE_INT:
	lt = (char*)  "i32";
	break;
    case TYPE_VOID:
	lt =  (char*) "void";
	break;
    default:
	lt =  (char*) "undef";
	break;
    }

    return lt;
}

// take a list of symbols*
const char *llvm_arglist(struct list *l)
{
    char  *tmp = (char*) "";
    int s = list_size(l);
	
    for (int i = 1; i <= s; ++i)
    {
	struct symbol *sy = (struct symbol*) list_get(l, i);
	asprintf(&tmp, "%s%s%s %%%s", tmp,
		 i == 1 ? "" : ", ",
		 llvm_type(sy->type), sy->name);
    }
    return tmp;
}

const char *llvm_arglist_type(struct list *l)
{
    char *tmp = "";
    int s = list_size(l);
	
    for (int i = 1; i <= s; ++i)
    {
	struct symbol *sy = (struct symbol*) list_get(l, i);
	asprintf(&tmp, "%s, %s ", tmp, llvm_type(sy->type));
    }
	
    return tmp;
}

const char *llvm_rvalue_eval(const struct expression *e)
{
    char *str;
    if (e->expression_type == EXPR_CONSTANT)
    {
	// the expression is a constant
	
	if (e->type->type == TYPE_INT)
	    asprintf(&str, "%d", e->constanti);
	else
	    asprintf(&str, "%f", e->constantf);
    }
    else
    {
	asprintf(&str, "%%t%u", e->var);
    }
	
    return str;
}

const char *llvm_lvalue_eval(const struct expression *e)
{
    char *str;
	
    assert (e->expression_type != EXPR_CONSTANT) ;
	
    if (e->expression_type == EXPR_POSTFIX)
    {
	asprintf(&str, "%%t%u", e->lvalue_var);
    }
    else if (e->expression_type == EXPR_SYMBOL)
    {
	asprintf(&str, "%%%s.stack", e->symbol->name);
    }
	
    return str;
}
