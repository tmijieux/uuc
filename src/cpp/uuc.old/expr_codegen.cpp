#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <assert.h>
#include <cstring>

#include "expression.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "error.hpp"
#include "program.hpp"
#include "llvm.hpp"

void expr_gen_code(struct expression * e)
{
    char *code = "";
    const char *typestr;
    const struct type* type;
    unsigned int s;

    unsigned int d;
    unsigned int d1;
    unsigned int d2;
	
    struct symbol *sy = e->symbol;
    e->lvalue_code = e->rvalue_code = "";
	
    if ( error_count() > 0 )
	return;
	
    switch(e->expression_type) {
    case EXPR_SYMBOL:
	typestr = llvm_type(e->type);
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%%t%u = load %s* %%%s.stack",
		 d, typestr, sy->name);
	e->rvalue_code = code;
	e->var = d;


	break;

    case EXPR_CONSTANT:
	
	break;

    case EXPR_MAP:
	
	break;
	
    case EXPR_REDUCE:

	break;

    case EXPR_FUNCALL_PARAMS:
	s = list_size(e->args);
	typestr = llvm_type(e->type);
	{
	    struct expression *arg =  list_get(e->args, 1);
	    char * tmp1 = arg->rvalue_code;
	    for (int i = 2; i <= s; ++i) {
		arg =  list_get(e->args, i);
		asprintf(&code,
			 "%s\n%s", tmp1, arg->rvalue_code);
		tmp1 = code;
	    }
	    code = tmp1;
	}
	if (e->type->type != TYPE_VOID)
	{
	    d1 = prgm_get_unique_id();
	    e->var = d1;
	    asprintf(&code,
		     "%s\n"  // code of arguments
		     "%%t%u = call %s @%s(",
		     code,
		     d1, typestr, sy->name);
	}
	else
	{
	    asprintf(&code,
		     "%s\n"  // code of arguments
		     "call %s @%s(",
		     code,
		     typestr, sy->name);
	}
	{
	    struct expression *arg;
	    for (int i = 1; i <= s; ++i) {
		arg =  list_get(e->args, i);
		asprintf(&code,"%s%s %s%c",
			 code,
			 llvm_type(arg->type),
			 llvm_rvalue_eval(arg),
			 i==s?')':',');
	    }
	}
	e->rvalue_code = code;
	break;
    case EXPR_FUNCALL: // no params
	typestr = llvm_type(e->type);
	if (e->type->type != TYPE_VOID)
	{
	    d1 = prgm_get_unique_id();
	    e->var = d1;
	    asprintf(&code,
		     "%%t%u = call %s @%s()",
		     d1, typestr, sy->name);
	}
	else
	{
	    asprintf(&code,
		     "call %s @%s()",
		     typestr, sy->name);
	}
	e->rvalue_code = code;
	break;
	
    case EXPR_POST_INC: // x ++
	d1 = prgm_get_unique_id();
	d2 = prgm_get_unique_id();
	typestr= llvm_type(e->type);
	
	asprintf(&code,
		 "%%t%u = load %s* %%%s.stack\n"
		 "%%t%u = add i32 %%t%u, 1\n"
		 "store %s %%t%u, %s* %%%s.stack",
		 d1, typestr, sy->name,
		 d2, d1,
		 typestr, d2, typestr, sy->name);
	e->rvalue_code = code;
	e->var = d1;
	break;
	
    case EXPR_POST_DEC: // x--
	d1 = prgm_get_unique_id();
	d2 = prgm_get_unique_id();
	typestr= llvm_type(e->type);
	
	asprintf(&code,
		 "%%t%u = load %s* %%%s.stack\n"
		 "%%t%u = sub i32 %%t%u, 1\n"
		 "store %s %%t%u, %s* %%%s.stack",
		 d1, typestr, sy->name,
		 d2, d1,
		 typestr, d2, typestr, sy->name);
	e->rvalue_code = code;
	e->var = d1;
	break;

    case EXPR_POSTFIX:
	typestr = llvm_type(e->operand1->type);
	d1 = prgm_get_unique_id();
	d2 = prgm_get_unique_id();
	d = prgm_get_unique_id();
	
	// TODO
	// this works only when the element is lvalue
	// need to add a line to load the value when it is a rvalue
	asprintf(&code,
		 "%s\n"
		 "%%t%u = load %s %s\n"
		 "%s\n"
		 "%%t%u = getelementptr %s %s, i32 0, %s %s",
		 e->operand1->rvalue_code,
		 d1, typestr,
		 llvm_rvalue_eval(e->operand1),
		 e->operand2->rvalue_code,
		 d2,
		 typestr,
		 llvm_rvalue_eval(e->operand1),
		 llvm_type(e->operand2->type),
		 llvm_rvalue_eval(e->operand2));
	
	e->lvalue_code = code;
	e->lvalue_var = d2;
	
	typestr = llvm_type(e->type);	
	asprintf(&code,
		 "%s\n"
		 "%%t%u = load %s* %%t%u",
		 e->lvalue_code, d, typestr, d2);
	e->rvalue_code = code;
	e->var = d;
	break;
	
    case EXPR_UNARY_MINUS:
	typestr = llvm_type(e->operand->type);
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%%t%u = sub %s 0, %s",
		 e->operand->rvalue_code,
		 d, typestr, llvm_rvalue_eval(e->operand));
	e->rvalue_code = code;
	e->var = d;
	break;
	
    case EXPR_PRE_INC:

	break;
	
    case EXPR_PRE_DEC:

	break;

    case EXPR_MULTIPLICATION:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = mul %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_DIVISION:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = div %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_ADDITION:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u =  add %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_SUBSTRACTION:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = sub %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_LOWER:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp slt %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_GREATER:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp sgt %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_LEQ:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp sle %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_GEQ:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp sge %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
    case EXPR_NEQ:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp ne %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_EQ:
	d = prgm_get_unique_id();
	asprintf(&code,
		 "%s\n"
		 "%s\n"
		 "%%t%u = icmp eq %s %s, %s",
		 e->operand1->rvalue_code,
		 e->operand2->rvalue_code,
		 d, llvm_type(e->operand1->type),
		 llvm_rvalue_eval(e->operand1),
		 llvm_rvalue_eval(e->operand2));
	e->var = d;
	e->rvalue_code = code;
	break;
	
    case EXPR_ASSIGNMENT:
	type = e->operand1->type;
	typestr = llvm_type(type);

	if ( e->operand1->expression_type == EXPR_SYMBOL ||
	     e->operand1->expression_type == EXPR_POSTFIX )
	{
	    if ( e->operand1->expression_type == EXPR_SYMBOL )
		assert ( e->operand1->symbol->symbol_type == SYM_VARIABLE );
		
	    // left operand is a variable
	    // N.B this can be an array
	    asprintf(&code,
		     "%s\n"
		     "store %s %s, %s* %s",
		     e->operand2->rvalue_code,
		     typestr,
		     llvm_rvalue_eval(e->operand2),
		     typestr,
		     llvm_lvalue_eval(e->operand1));
	}
	
	if ( e->operand1->expression_type == EXPR_POSTFIX )
	{ // if the assignment is in an array
	  // add the code to get the pointer to the right element
	  // of the array in front:
	    asprintf(&code, "%s\n%s", e->operand1->lvalue_code, code);
	}
	
	e->rvalue_code = code;
    }
}
