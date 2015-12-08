#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "expression.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "error.hpp"
#include "expr_codegen.hpp"
#include "llvm.hpp"

enum class {   LOL, B,A	};

/**
 * create an expression out of other expressions or constants
 * and type-check it
 */
struct expression *expr_new(enum expression_type ext, ...)
{
    struct symbol *sy = NULL;
    struct expression *expr = (struct expression*) malloc(sizeof(*expr));
    int first_operand_valid = 0, second_operand_valid = 0;
    struct list *args = NULL; // list of expression
	
    expr->expression_type = ext;
    expr->lvalue_code = expr->rvalue_code = expr->source_code = "";

    va_list ap;
    va_start(ap, ext);
	
    switch ( ext ) {
    case EXPR_SYMBOL:
	sy = va_arg(ap, struct symbol*);
	expr->type = sy->type;
	expr->symbol = sy;
	expr->source_code = strdup(sy->name);
	expr->value = sy->value;
	break;

    case EXPR_CONSTANT:
	expr->type = va_arg(ap, struct type*);
	if ( expr->type->type == TYPE_INT )
	{
	    expr->constanti = va_arg(ap, int);
	    expr->value = llvm::ConstantFP::get(
		LLVM::context,
		llvm::APFloat(expr->constantf)
	    );

	}
	else if ( expr->type->type == TYPE_FLOAT )
	{
	    expr->constantf = (float) va_arg(ap, double);
	    expr->value = llvm::ConstantFP::get(
		LLVM::context,
		llvm::APFloat(expr->constantf)
	    );
	    // there can't be a float in va_arg
	}
	else
	{
	    fatal_error("unexpected parse error %s:%d\n", __FILE__, __LINE__);
	}
	break;

    case EXPR_MAP:
	expr->operand1 = va_arg(ap, struct expression *);
	expr->operand2 = va_arg(ap, struct expression *);
	
	if (expr->operand1->type->type != TYPE_FUNCTION)
	{
	    error("First operand to map operator must be a function.\n");
	}
	else
	{
	    if (expr->operand1->type->function_type.argc != 1)
	    {
		error("Map first operand should take exactly one parameter.\n");
	    }
	    else
	    {
		first_operand_valid = 1;
	    }
	}
	
	if (expr->operand2->type->type != TYPE_ARRAY)
	{
	    error("Second operand to map operator must be an array.\n");
	}
	else
	{
	    second_operand_valid = 1;
	}
	
	if (first_operand_valid && second_operand_valid)
	{
	    struct symbol *tmp;
	    // the function 1st (and last) argument
	    tmp = list_get(expr->operand1->type->function_type.args, 1);
	    const struct type *ty = tmp->type; // its type
		
	    if ( !type_equal(ty, expr->operand2->type->array_type.values) )
	    { // must be the same as the array values
		error("map operands: type mismatch\n");
	    }
	    expr->type = type_new_array_type(expr->operand1->
					     type->function_type.
					     return_value, -1);
	}
	else
	{
	    expr->type = type_generic;
	}
	asprintf(&expr->source_code, "map(%s, %s)",
		 expr->operand1->source_code,
		 expr->operand2->source_code);
	break;
	
    case EXPR_REDUCE:
	expr->operand1 = va_arg(ap, struct expression *);
	expr->operand2 = va_arg(ap, struct expression *);
	
	first_operand_valid = second_operand_valid = 0;
	
	if (expr->operand1->type->type != TYPE_FUNCTION)
	{
	    error("First operand to reduce operator must be a function.\n");
	}
	else
	{
	    if (expr->operand1->type->function_type.argc != 2)
	    {
		error("reduce first operand should take exactly "
		      "two parameters.\n");
	    }
	    else
	    {
		first_operand_valid = 1;
	    }
	}
	
	if (expr->operand2->type->type != TYPE_ARRAY)
	{
	    error("Second operand to reduce operator must be an array.\n");
	}
	else
	{
	    second_operand_valid = 1;
	}
	
	if (first_operand_valid && second_operand_valid)
	{
	    const struct type *ty1, *ty2;
	    ty1 = ((struct symbol*) list_get(expr->operand1->type->
					     function_type.args, 1))
		->type;
		
	    ty2 = ((struct symbol*) list_get(expr->operand1->type->
					     function_type.args, 2))
		->type;

	    if ( !type_equal(ty1, ty2) )
	    {
		error("reduce first operand: invalid parameters\n");
	    }
		
	    if ( !type_equal(ty1, expr->operand2->type->array_type.values) )
	    {
		error("reduce operands: type mismatch\n");
	    }

	    expr->type =  expr->operand1->type->function_type.return_value;
	}
	else
	{
	    expr->type = type_generic;
	}
	break;

    case EXPR_FUNCALL_PARAMS:
	args = va_arg(ap, struct list*); // list of expressions
	expr->args = args;
	// NO BREAK HERE ON PURPOSE !!
    case EXPR_FUNCALL: // no params
	sy = va_arg(ap, struct symbol*);
	expr->symbol = sy;
	if (sy->type->type != TYPE_FUNCTION)
	{
	    fatal_error("'%s' is not a function.\n", sy->name);
	    expr->type = type_generic;
	}
	else
	{
	    struct list *proto =  sy->type->function_type.args;
	    unsigned int s = list_size(proto);
	    if ((args == NULL && s != 0) || // check argument count
		(args != NULL && list_size(args) != s))
	    {
		error("%s: illegal number of arguments.\n", sy->name);
	    }
	    else
	    {
		for (unsigned int i = 1; i <= s; ++i)
		{
		    const struct type *tparg;	// type in definition
		    tparg = ((struct symbol*) list_get(proto, i))->type;
			
		    const struct type *targ; // type given
		    targ = ((struct expression*) list_get(args, i))->type;
			
		    if ( !type_equal(tparg, targ) )
		    {
			error("%s(): argument %d has invalid type.\n"
			      "expected ",  sy->name, i);
			printf("\e[92m");
			type_print_line(tparg);
			printf("\e[96m but \e[93m");
			type_print_line(targ);
			printf ("\e[96m was given.\e[39m\n");
		    }
		}
	    }
	    expr->type = sy->type->function_type.return_value;
	}
	break;

    case EXPR_POSTFIX:
	expr->operand1 = va_arg(ap, struct expression*);
	expr->operand2 = va_arg(ap, struct expression*);
	
	if ( expr->operand1->type->type != TYPE_ARRAY &&
	     expr->operand1->type->type != TYPE_GENERIC )
	{
	    error("There must be an array prior to []\n");
	    expr->type = type_generic;
	}
	else
	{
	    expr->type = expr->operand1->type->array_type.values;
	}
	
	if ( expr->operand2->type->type != TYPE_INT &&
	     expr->operand2->type->type != TYPE_GENERIC )
	{
	    error("Array index have to be integer\n");
	}
	
	break;
	
    case EXPR_UNARY_MINUS:
	expr->operand = va_arg(ap, struct expression*);

	expr->type = expr->operand->type;
	
	if (expr->type->type != TYPE_INT &&
	    expr->type->type != TYPE_FLOAT &&
	    expr->type->type != TYPE_GENERIC)
	{
	    error("unary minus does not apply to type ");
	    printf("\e[93m");
	    type_print_line(expr->type);
	    printf("\e[39m\n");
	    expr->type = type_generic;
	}

	break;

    case EXPR_POST_INC: // x ++
    case EXPR_POST_DEC: // x --
    case EXPR_PRE_INC:  // ++ x
    case EXPR_PRE_DEC:  // -- x

	expr->operand = va_arg(ap, struct expression*);
	expr->type = expr->operand->type;
	expr->symbol = expr->operand->symbol;

	if ( expr->operand->expression_type != EXPR_POSTFIX &&
	     expr->operand->expression_type != EXPR_SYMBOL )
	{
	    error("increment/decrement cannot be applied\nto non left"
		  " value \e[4m\e[33m'%s'\e[0m\n",  expr->operand->source_code);
	    expr->type = type_generic;
	}
	else if ( expr->type->type != TYPE_INT &&
		  expr->type->type != TYPE_GENERIC )
	{
	    error("cannot increment/decrement on type ");
	    printf("\e[93m");
	    type_print_line(expr->type);
	    printf("\e[39m\n");
	    expr->type = type_generic;
	}

	asprintf(&expr->source_code, "%s %s",
		 expr->operand->source_code,
		 "++"); // FIXME must depent on the expression_type
	break;

    case EXPR_MULTIPLICATION:
    case EXPR_DIVISION:
    case EXPR_ADDITION:
    case EXPR_SUBSTRACTION:
    case EXPR_LOWER:
    case EXPR_GREATER:
    case EXPR_LEQ:
    case EXPR_GEQ:
    case EXPR_NEQ:
    case EXPR_EQ:
	expr->operand1 = va_arg(ap, struct expression *);
	expr->operand2 = va_arg(ap, struct expression *);

	if ( (expr->operand1->type->type != TYPE_INT &&
	      expr->operand1->type->type != TYPE_FLOAT &&
	      expr->operand1->type->type != TYPE_GENERIC  ) ||
	     (expr->operand2->type->type != TYPE_INT &&
	      expr->operand2->type->type != TYPE_FLOAT &&
	      expr->operand2->type->type != TYPE_GENERIC  ) ||
	     !type_equal(expr->operand1->type,expr->operand2->type) )
	{
	    error("operation impossible: have ");
	    printf("\e[93m"); // yellow
	    type_print_line( expr->operand1->type );
	    printf("\e[96m and \e[93m"); // cyan then yellow
	    type_print_line( expr->operand2->type);
	    printf("\e[39m\n"); // normal
	    expr->type = type_generic;
	}
	else
	{
	    expr->type = type_int; // int for boolean
	}

	asprintf(&expr->source_code, "%s %s %s",
		 expr->operand1->source_code,
		 "+",  // FIXME must depent on the expression_type
		 expr->operand2->source_code);
	break;

    case EXPR_ASSIGNMENT:
	expr->operand1 = va_arg(ap, struct expression *);
	expr->operand2 = va_arg(ap, struct expression *);
	
	if ( !type_equal(expr->operand1->type, expr->operand2->type) )
	{
	    error("assignment: type mismatch\n");
	    expr->type = type_generic;
	}
	else
	{
	    expr->type = expr->operand1->type;
	}

	switch ( expr->operand1->expression_type ) {
	case EXPR_PRE_INC:
	case EXPR_PRE_DEC:
	case EXPR_POST_INC:
	case EXPR_POST_DEC:
	case EXPR_UNARY_MINUS:
	case EXPR_REDUCE:
	case EXPR_CONSTANT:
	    error("assignment: invalid left value\n");
	    expr->type = type_generic;
	    break;
	default:
	    break;
	}
	break;
    }
	
    expr_gen_code(expr);
    va_end(ap);
    return expr;
}
