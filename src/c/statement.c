#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include "statement.h"
#include "program.h"
#include "symbol.h"
#include "codegen.h"
#include "color.h"
#include "error.h"

static const char *str_concat_list(const struct list *l,
				   const char * (*get_str__)(void *))
{
    if (!l)
	return "";
    
    int s = list_size(l);
    char *code = "";
    
    for (int i = 1; i <= s; ++i)
	asprintf(&code, "%s%s", code, get_str__(list_get(l, i)));

    return code;
}

static const char *stmt_get_code(void * st)
{
    return ((struct statement*)st)->code;
}

static const char *symb_get_init_code(void * st)
{
    return ((struct symbol*)st)->variable.init_code;
}

const char *decl_init_list(const struct list *l)
{
	return str_concat_list(l, &symb_get_init_code);
}

/*******************/

static 
struct statement *stmt_new(enum statement_type st)
{
    struct statement *stmt = calloc(sizeof*stmt, 1);
    stmt->code = "";
    stmt->statement_type = st;
    return stmt;
}

struct statement *stmt_expression(const struct expression *expr)
{
    struct statement *stmt = stmt_new(STMT_EXPR);
    stmt->expr = expr;
    if ( expr ) {
	    expr_cg(expr);
	    stmt->code = expr->rvalue_code;
    }
    return stmt;
}

struct statement *stmt_compound(const struct list *decl,
				const struct list *stmts)
{
    struct statement *stmt = stmt_new(STMT_COMPOUND);

    stmt->decl_list = decl;
    stmt->stmt_list = stmts;
    char *code;
    asprintf(&code, "%s%s",
	     str_concat_list(decl, &symb_get_init_code),
	     str_concat_list(stmts, &stmt_get_code));
    stmt->code = code;
    
    return stmt;
}

struct statement *stmt_if(const struct expression *cond,
			  const struct statement *then)
{
    struct statement *stmt = stmt_new(STMT_SELECTION);
    unsigned int d = prgm_get_unique_id();
    char *code;

    if ( !expr_is_test( cond ) )
    {
	    const struct expression *zero = expr_constant( type_int, 0);
	    cond = expr_neq( cond, zero);
    }
    
    expr_cg(cond);

    asprintf(&code,
	     "br label %%cond%d\n" // end previous block with a br instruction
	     "cond%d:\n"
	     "%s"
	     "br i1 %s, label %%then%u, label %%end%u\n"
	     "then%u:\n"
	     "%s"
	     "br label %%end%u\n"
	     "end%u:\n",
	     d, d, cond->rvalue_code,
	     expr_cg_rvalue_eval(cond), d, d,
	     d, then->code, d,  d);

    stmt->code = code;
    return stmt;
}

struct statement *stmt_if_else(const struct expression *cond,
			       const struct statement *then,
			       const struct statement *eelse)
{
    struct statement *stmt = stmt_new(STMT_SELECTION);
    unsigned int d = prgm_get_unique_id();
    char *code;

    if ( !expr_is_test( cond ) )
    {
	    const struct expression *zero = expr_constant( type_int, 0);
	    cond = expr_neq( cond, zero);
    }

    expr_cg(cond);
    
    asprintf(&code,
	     "br label %%cond%d\n" // end previous block with a br instruction
	     "cond%d:\n"
	     "%s"
	     "br i1 %s, label %%then%u, label %%else%u\n"
	     "then%u:\n"
	     "%s"
	     "br label %%end%u\n"
	     "else%u:\n"
	     "%s"
	     "br label %%end%u\n"
	     "end%u:\n",
	     d, d, cond->rvalue_code,
	     expr_cg_rvalue_eval(cond), d, d,
	     d,then->code, d, d,
	     eelse->code, d, d);
    stmt->code = code;
    return stmt;
}

struct statement *stmt_for(const struct expression *init,
			   const struct expression *cond,
			   const struct expression* next,
			   const struct statement *body)
{
    struct statement *stmt = stmt_new(STMT_ITERATION);
    unsigned int d = prgm_get_unique_id();
    char *code;

    if ( !expr_is_test( cond ) )
    {
	    const struct expression *zero = expr_constant( type_int, 0);
	    cond = expr_neq( cond, zero );
    }
    
    expr_cg(init);
    expr_cg(cond);
    expr_cg(next);
    
    asprintf(&code,
	     "%s"
	     "br label %%cond%d\n"
	     "cond%d:\n"
	     "%s"
	     "br i1 %s, label %%start%u, label %%end%u\n"
	     "start%u:\n"
	     "%s"
	     "%s"
	     "br label %%cond%u\n"
	     "end%u:\n",
	     init->rvalue_code, d, d, cond->rvalue_code,
	     expr_cg_rvalue_eval(cond), d, d, d,  body->code,
	     next->rvalue_code, d, d); 
    stmt->code = code;   
    return stmt;
}

struct statement *stmt_while(const struct expression *cond,
			     const struct statement *body)
{
    struct statement *stmt = stmt_new(STMT_ITERATION);
    unsigned int d = prgm_get_unique_id();
    char *code;

    if ( !expr_is_test( cond ) )
    {
	    const struct expression *zero = expr_constant(type_int, 0);
	    cond = expr_neq( cond, zero);
    }

    expr_cg(cond);
    
    asprintf(&code,
	     "br label %%cond%d\n" // end previous block with a br instruction
	     "cond%d:\n"
	     "%s"
	     "br i1 %s, label %%loop%u, label %%end%u\n"
	     "loop%u:\n"
	     "%s"
	     "br label %%cond%u\n"
	     "end%u:\n",
	     d, d, cond->rvalue_code,
	     expr_cg_rvalue_eval(cond), d, d,   d,
	     body->code, d, d);
    stmt->code = code;        
    return stmt;
}

struct statement *stmt_do_while(const struct expression *cond,
				const struct statement *body)
{
    struct statement *stmt = stmt_new(STMT_ITERATION);
    unsigned int d = prgm_get_unique_id();
    char *code;


    if ( !expr_is_test( cond ) )
    {
	    const struct expression *zero = expr_constant( type_int, 0);
	    cond = expr_neq( cond, zero);
    }

    expr_cg(cond);
    
    asprintf(&code,
    	     "br label %%start%d\n"
    	     "start%u:\n"
    	     "%s"
    	     "%s"
    	     "br i1 %s, label %%start%u, label %%end%u\n"
    	     "end%u:\n",
    	     d, d, body->code, cond->rvalue_code,
	     expr_cg_rvalue_eval(cond), d, d, d);
    stmt->code = code;
    return stmt;
}

struct statement *stmt_return_void(void)
{
    struct statement *stmt = stmt_new(STMT_JUMP);
    if ( last_function_return_type->type != TYPE_VOID )
    {
	error("return value can't be void. expected %s\n",
	      color("yellow", type_printable(last_function_return_type)));
    }
    
    stmt->code = "ret void\n";
    return stmt;
}

struct statement *stmt_return(const struct expression *expr)
{
    struct statement *stmt = stmt_new(STMT_JUMP);
    stmt->expr = expr;
    char *code;

    if ( last_function_return_type == type_void )
    {
	error("returning non void value\n");
    }
    else if  (!type_equal(last_function_return_type, expr->type))
    {
	if ( type_is_basic(last_function_return_type) &&
	     type_is_basic(expr->type))
	{
	    warning("return statement makes an implicit cast\n");
	    expr = expr_cast( expr, last_function_return_type);
	}
	else
	{
	    error("returning type %s %s %s %s",
		  color("yellow", type_printable(expr->type)),
		  color("light blue", "while"),
		  color("green", type_printable(last_function_return_type)),
		  color("light blue", "was expected.\n"));
	}
    }

    expr_cg(expr);

    asprintf(&code,
	     "%s"
	     "ret %s %s\n",
	     expr->rvalue_code, type_cg(expr->type),
	     expr_cg_rvalue_eval(expr));
    
    stmt->code = code;
    return stmt;
}


