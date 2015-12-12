#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <assert.h>

#include "expression.h"
#include "symbol.h"
#include "type.h"
#include "error.h"
#include "program.h"
#include "codegen.h"
#include "expr_codegen.h"

inline void expr_cg(const struct expression *e)
{
    if ( error_count() > 0 )
	return;
    e->codegen((struct expression*) e); // ON PURPOSE UNCONSTING
}

// -- helpers -- >>


static char* new_register(void)
{
	char *reg;
	asprintf(&reg, "%%t%u", prgm_get_unique_id());
	return reg;
}

static void new_registers(int n, char *tab[])
{
	for (int i = 0; i < n; i++)
		tab[i] = new_register();
}

static void
expr_cg_operation(struct expression *e, const char *op, const char *prefix)
{
    expr_cg(e->left_operand);
    expr_cg(e->right_operand);
	
    int d = prgm_get_unique_id();
    asprintf(&e->rvalue_code,
	     "%s"
	     "%s"
	     "%%t%u = %s%s %s %s, %s\n",
	     e->left_operand->rvalue_code,
	     e->right_operand->rvalue_code,
	     d,
	     prefix, op,
	     type_cg(e->left_operand->type),
	     expr_cg_rvalue_eval(e->left_operand),
	     expr_cg_rvalue_eval(e->right_operand));
    e->var = d;
}

static void expr_cg_xcrementimpl(struct expression *e, const char *op, int d1)
{
	
    expr_cg(e->operand);

    const char *typestr = type_cg(e->type);
    asprintf(&e->rvalue_code,
	     "%s"
	     "%%t%u = %s i32 %s, 1\n"
	     "store %s %%t%u, %s* %s\n",
	     e->operand->rvalue_code,
	     d1, op,
	     expr_cg_rvalue_eval(e->operand),
	     typestr, d1, typestr,
	     expr_cg_lvalue_eval(e->operand));
}

static void expr_cg_post_xcrement(struct expression *e, const char *op)
{
    unsigned int d1 = prgm_get_unique_id();
    expr_cg_xcrementimpl(e, op, d1);
    e->var = e->operand->var;
}

static void expr_cg_pre_xcrement(struct expression *e, const char *op)
{
    unsigned int d1 = prgm_get_unique_id();
    expr_cg_xcrementimpl(e, op, d1);
    e->var = d1;
}

// << -- helpers end --

void expr_cg_xcrement(struct expression *e)
{
    switch ( e->expression_type ) {
    case EXPR_POST_INC: // x ++
	expr_cg_post_xcrement(e, "add");
	break;
    case EXPR_POST_DEC: // x--
	expr_cg_post_xcrement(e, "sub");
	break;
    case EXPR_PRE_INC:
	expr_cg_pre_xcrement(e, "add");
	break;
    case EXPR_PRE_DEC:
	expr_cg_pre_xcrement(e, "sub");
	break;
    default:
	internal_error("expr_cg_xcrement: default clause reached");
    }
}


#define expr_cg_xoperation_case(TYPE_, OPNAME_, PREFIX_)		        \
    case EXPR_##TYPE_:						        	\
        expr_cg_operation(e, OPNAME_, (e->type == type_float)?"f":PREFIX_);	\
        break

void expr_cg_xoperation(struct expression *e)
{
    switch ( e->expression_type ) {
	expr_cg_xoperation_case(MULTIPLICATION, "mul", "");
	expr_cg_xoperation_case(DIVISION, "div", "s");
	expr_cg_xoperation_case(ADDITION, "add", "");
	expr_cg_xoperation_case(SUBSTRACTION, "sub", "");
	expr_cg_xoperation_case(LOWER, "cmp slt", "i");
	expr_cg_xoperation_case(GREATER, "cmp slt", "i");
	expr_cg_xoperation_case(LEQ, "cmp sle", "i");
	expr_cg_xoperation_case(GEQ, "cmp sge", "i");
	expr_cg_xoperation_case(NEQ, "cmp ne", "i");
	expr_cg_xoperation_case(EQ, "cmp eq", "i");
	
    default:
	internal_error("expr_cg_operation: default clause reached");
	break;
    }
}
#undef expr_cg_xoperation_case


const char *expr_cg_rvalue_eval(const struct expression *e)
{
    char *str;
    if (e->expression_type == EXPR_CONSTANT)
    {
	if ( e->type == type_int )
	{
	    asprintf(&str, "%d", e->constanti);
	}
	else if ( e->type == type_float )
	{
	    double tmp = (double) e->constantf;
	    asprintf(&str, "%#018lx",  *(uint64_t*) (&tmp));
	}
	else if ( e->type == type_long )
	{
	    asprintf(&str, "%ld", e->constantl);
	}
    }
    else if ( e->expression_type == EXPR_ASSIGNMENT )
    {
	return expr_cg_rvalue_eval( e->right_operand );
	// evaluation of an assignment if the evaluation
	// of its right operand, nice recursive style
    }
    else
    {
	asprintf(&str, "%%t%u", e->var);
    }
    return str;
}

const char *expr_cg_lvalue_eval(const struct expression *e)
{
    char *str;
    
    assert (e->expression_type != EXPR_CONSTANT) ;
    
    if ( e->expression_type == EXPR_POSTFIX ||
	 e->expression_type == EXPR_ARRAY_SIZE )
    {
	asprintf(&str, "%%t%u", e->lvalue_var);
    }
    else if ( e->expression_type == EXPR_SYMBOL )
    {
	return symbol_fully_qualified_name(e->symbol);
    }
    
    return str;
}

void expr_cg_map(struct expression *e)
{
    char *syname, *rettab;

    expr_cg(e->fun);
    expr_cg(e->array);

    const char *funtypestr = type_cg(e->fun->type);
    const char *paramarraytypestr = type_cg(e->array->type);
    const char *retarraytypestr = type_cg(e->type);


    int d1 ,d2, d3, d4, d5, d6;
    d1 = prgm_get_unique_id(); d2 = prgm_get_unique_id();
    d3 = prgm_get_unique_id(); d4 = prgm_get_unique_id();
    d5 = prgm_get_unique_id(); d6 = prgm_get_unique_id();

    // RETURN ARRAY:
    asprintf(&rettab, "map%u", d1);

    // printf("type map ret %s\n\n\n\n\n", type_printable(e->type));
    struct symbol *rettab_symb = symbol_new(rettab, e->type);
    rettab_symb->suffix = "mapcg";
    symb_cg(rettab_symb);

    // MAP ARGS:
    asprintf(&syname, "mapcont%u", d2);

    uint64_t mpc = // map call type
	(type_size(e->array->type->array_type.values) == 4 ? 0L : 1L)
	+ 2 *
	(type_size(e->type->array_type.values) == 4 ? 0L : 1L);
	
    char *mapconttypestr;
    asprintf(&mapconttypestr, "{i64, %s, %s, %s}",
	     paramarraytypestr, retarraytypestr, funtypestr);
	
    char *mapcont_code;
    asprintf(&mapcont_code,
	     "%%%s.mapcg = alloca %s\n" // struct mapcont
		 
	     "%%t%u = getelementptr %s* %%%s.mapcg, i64 0, i32 0\n"
	     "store i64 %ld, i64* %%t%u\n" // set map_call_type

	     "%s"
	     "%%t%u = getelementptr %s* %%%s.mapcg, i64 0, i32 1\n"
	     "store %s %s, %s* %%t%u\n" // set param array

	     "%%t%u = getelementptr %s* %%%s.mapcg, i64 0, i32 2\n"
	     "%%t%u = load %s* %%%s.mapcg\n"
	     "store %s %%t%u, %s* %%t%u\n" // set return array
		 
	     "%%t%u = getelementptr %s* %%%s.mapcg, i64 0, i32 3\n"
	     "store %s @%s, %s* %%t%u\n" // set method

	     "%%t%u = bitcast %s* %%%s.mapcg to i8*\n"
	     "call void @map(i8* %%t%u)\n", // call map
		 
	     syname, 
	     mapconttypestr,    // struct mapcont

	     d1, mapconttypestr,  syname,
	     mpc,
	     d1, // set map_call_type

	     e->array->rvalue_code,
	     d2, mapconttypestr, syname, paramarraytypestr,
	     expr_cg_rvalue_eval(e->array),
	     paramarraytypestr, d2,// set param array

	     d3, mapconttypestr, syname,

	     d5, retarraytypestr, rettab,
	     retarraytypestr, d5,
	     retarraytypestr, d3,// set return array

	     d4, mapconttypestr, syname, funtypestr,
	     e->fun->symbol->name,
	     funtypestr, d4, // set method

	     d6, mapconttypestr, syname, d6); // call map

    symb_cg(rettab_symb);
    asprintf(&mapcont_code,
	     "%s%s%s",
	     rettab_symb->variable.alloc_code,
	     rettab_symb->variable.init_code,
	     mapcont_code);

//	printf("rettab init code %s\n\n\n\n", rettab_symb->variable.init_code);
	
    e->rvalue_code = mapcont_code;
    e->var = d5;
}

void expr_cg_reduce(struct expression *e)
{
    char *syname, *ret;

    expr_cg(e->fun);
    expr_cg(e->array);

    const char *funtypestr = type_cg(e->fun->type);
    const char *paramarraytypestr = type_cg(e->array->type);
    const char *rettypestr = type_cg(e->type);

    int d1 ,d2, d3, d4, d5, d6;
    d1 = prgm_get_unique_id(); d2 = prgm_get_unique_id();
    d3 = prgm_get_unique_id(); d4 = prgm_get_unique_id();
    d5 = prgm_get_unique_id(); d6 = prgm_get_unique_id();

    // RETURN ARRAY:
    asprintf(&ret, "redret%u", d1);
    struct symbol *ret_symb = symbol_new(ret, e->type);
    ret_symb->suffix = "redcg";
    symb_cg(ret_symb);

    // MAP ARGS:
    asprintf(&syname, "redcont%u", d2);

    char *redconttypestr;
    asprintf(&redconttypestr, "{i64, %s, %s*, %s}",
	     paramarraytypestr, rettypestr, funtypestr);
	
    char *redcont_code;
    asprintf(&redcont_code,
	     "%%%s.redcg = alloca %s\n" // struct redcont
		 
	     "%%t%u = getelementptr %s* %%%s.redcg,"
	     " i64 0, i32 0\n"
	     "store i64 %ld, i64* %%t%u\n" // set red_call_type

	     "%s"
	     "%%t%u = getelementptr %s* %%%s.redcg,"
	     " i64 0, i32 1\n"
	     "store %s %s, %s* %%t%u\n" // set param array

	     "%%t%u = getelementptr %s* %%%s.redcg, i64 0, i32 2\n"
	     "store %s* %%%s.redcg, %s** %%t%u\n" // set return value
		 
	     "%%t%u = getelementptr %s* %%%s.redcg, i64 0, i32 3\n"
	     "store %s @%s, %s* %%t%u\n" // set method

	     "%%t%u = bitcast %s* %%%s.redcg to i8*\n"
	     "call void @reduce(i8* %%t%u)\n"  // call red

		 
	     "%%t%u = load %s* %%%s.redcg\n", //retval
		 
	     syname, 
	     redconttypestr,    // struct redcont

	     d1, redconttypestr,  syname,
	     type_size(e->array->type->array_type.values) == 4 ? 0L : 1L,
	     d1, // set red_call_type

	     e->array->rvalue_code,
	     d2, redconttypestr, syname, paramarraytypestr,
	     expr_cg_rvalue_eval(e->array),
	     paramarraytypestr, d2,// set param array

	     d3, redconttypestr, syname,
	     rettypestr, ret, rettypestr, d3,// set return value

	     d4, redconttypestr, syname, funtypestr,
	     e->fun->symbol->name,
	     funtypestr, d4, // set method

	     d6, redconttypestr, syname, d6,  // call red

	     d5, rettypestr, ret); //retval


    asprintf(&redcont_code,
	     "%s%s%s",
	     ret_symb->variable.alloc_code,
	     ret_symb->variable.init_code,
	     redcont_code);

    e->rvalue_code = redcont_code;
    e->var = d5;
}

void expr_cg_array_size(struct expression *e)
{
    char *code;
    int d1 = prgm_get_unique_id();
    int d2 = prgm_get_unique_id();

    expr_cg(e->array);

    asprintf(&code,
	     "%s"
	     "%%t%u = getelementptr %s %s, i64 0, i32 0\n",
	     e->array->rvalue_code,
	     d1, type_cg(e->array->type),
	     expr_cg_rvalue_eval(e->array)
	);

    e->lvalue_code = code;
    e->lvalue_var = d1;
	
    asprintf(&code,
	     "%s"
	     "%%t%u = load i64* %%t%u\n",
	     code,
	     d2, d1);
	
    e->rvalue_code = code;
    e->var = d2;
}

void expr_cg_symbol(struct expression *e)
{
    const char *typestr = type_cg(e->type);
    int d = prgm_get_unique_id();

    char *code;
    asprintf(&code,
	     "%%t%u = load %s* %s\n",
	     d, typestr, expr_cg_lvalue_eval(e));
	
    e->rvalue_code = code;
    e->var = d;
}

void expr_cg_constant(struct expression *e)
{
    if ( e->type == type_int )
	asprintf(&e->rvalue_code,
		 "; no rvalue code: constant = %d\n",
		 e->constanti);
	
    else if ( e->type == type_float )
	asprintf(&e->rvalue_code,
		 "; no rvalue code: constant = %.10g\n",
		 e->constantf);
	
    else if ( e->type == type_long )
	asprintf(&e->rvalue_code,
		 "; no rvalue code: constant = %ld\n",
		 e->constantl);

    // TODO handle for float/ simplify rvalue_eval
}


void expr_cg_funcall_params(struct expression *e)
{
    char *params_code = "";
    char *params_val = "";
    char *call_code = "";
    
    int s = list_size(e->args);
    for (int i = 1; i <= s; ++i)
    {
	struct expression *arg;			
	arg = list_get(e->args, i);
	expr_cg(arg);
	asprintf(&params_code, "%s%s",  params_code, arg->rvalue_code);
	asprintf(&params_val,"%s%s %s%s", params_val, type_cg(arg->type),
		 expr_cg_rvalue_eval(arg),  i==s?"":",");
    }

    asprintf(&call_code, "call %s @%s(%s)\n",
	     type_cg(e->type), e->symbol->name, params_val);
    
    if ( e->type != type_void )
    {
	int d = prgm_get_unique_id();
	e->var = d;
	asprintf(&call_code, "%%t%u = %s", d, call_code);
    }

    asprintf(&e->rvalue_code, "%s%s", params_code, call_code);
}

void expr_cg_funcall(struct expression *e)
{
    e->args = list_new(0);
    expr_cg_funcall_params(e);
}

void expr_cg_postfix(struct expression *e)
{
    const char *typestr = type_cg(e->array->type);
    int d2 = prgm_get_unique_id();
    int d3 = prgm_get_unique_id();
	
    assert ( e->array != e->index );
    expr_cg(e->array);
    expr_cg(e->index);
	
    char *code;		
    asprintf(&code,
	     "%s"
	     "%s"
	     "%%t%u = getelementptr %s %s, i64 0, i32 1, %s %s\n",
	     e->array->rvalue_code,
	     e->index->rvalue_code,
	     d2, typestr,
	     expr_cg_rvalue_eval(e->array),
	     type_cg(e->index->type),
	     expr_cg_rvalue_eval(e->index));
	
    e->lvalue_code = code;
    e->lvalue_var = d2;

    typestr = type_cg(e->type);	
    asprintf(&code,
	     "%s"
	     "%%t%u = load %s* %%t%u\n",
	     e->lvalue_code, d3, typestr, d2);
    e->rvalue_code = code;
    e->var = d3;	
}

void expr_cg_unary_minus(struct expression *e)
{//TODO make expr_unary_minus return expresion_substraction(epxr_constant,
    //and eventually remove this function
    const char *typestr = type_cg(e->operand->type);
    int d = prgm_get_unique_id();
	
    expr_cg(e->operand);
	
    char *code;
    asprintf(&code,
	     "%s"
	     "%%t%u = sub %s 0, %s\n",
	     e->operand->rvalue_code,
	     d, typestr, expr_cg_rvalue_eval(e->operand));
	
    e->rvalue_code = code;
    e->var = d;
}

void expr_cg_assignment(struct expression *e)
{
    const struct type *type = e->left_operand->type;
    const char *typestr = type_cg(type);

    expr_cg(e->left_operand);
    expr_cg(e->right_operand);
	
    if ( e->left_operand->expression_type == EXPR_SYMBOL ||
	 e->left_operand->expression_type == EXPR_POSTFIX ||
	 e->left_operand->expression_type == EXPR_ARRAY_SIZE )
    {
	if ( e->left_operand->expression_type == EXPR_SYMBOL )
	{
	    assert ( e->left_operand
		     ->symbol->symbol_type == SYM_VARIABLE );
	}
	    
	// left operand is a variable
	// N.B this can be an array
	asprintf(&e->rvalue_code,
		 "%s"
		 "store %s %s, %s* %s\n",
		 e->right_operand->rvalue_code,
		 typestr,
		 expr_cg_rvalue_eval(e->right_operand),
		 typestr,
		 expr_cg_lvalue_eval(e->left_operand));
    }
	
    if ( e->left_operand->expression_type == EXPR_POSTFIX ||
	 e->left_operand->expression_type == EXPR_ARRAY_SIZE )
    { // if the assignment is in an array
	// add the code to get the pointer to the right element
	// of the array in front:
	// TODO add emtpy lvalue code for other expression
	asprintf(&e->rvalue_code, "%s%s",
		 e->left_operand->lvalue_code,
		 e->rvalue_code);
    }
}

void expr_cg_fpsicast(struct expression *e)
{
    char *cast_instr = "unimplemented cast";

    expr_cg(e->operand);
		
    if ( e->operand->type == type_float &&
	 type_is_integer( e->target_type ) )
    {
	cast_instr = "fptosi";
    }
    else if ( type_is_integer( e->operand->type ) &&
	      e->target_type == type_float )
    {
	cast_instr = "sitofp";
    }
	
    const char *typestr = type_cg(e->operand->type);
    int d = prgm_get_unique_id();

    char *code;
    asprintf(&code,
	     "%s"
	     "%%t%u = %s %s %s to %s\n",
	     e->operand->rvalue_code,
	     d, cast_instr,typestr,
	     expr_cg_rvalue_eval(e->operand),
	     type_cg(e->target_type));
    e->rvalue_code = code;
    e->var = d;
}

static void expr_cg_constant_expression(
    struct expression *e, const char *opname)
{
    int d = prgm_get_unique_id();
	
    expr_cg(e->operand);
		
    char *code;
    asprintf(&code,
	     "%s"
	     "%%t%u = %s %s %s to %s\n",
	     e->operand->rvalue_code,
	     d, opname,
	     type_cg(e->operand->type),
	     expr_cg_rvalue_eval(e->operand),
	     type_cg(e->target_type)
	);
    e->rvalue_code = code;
    e->var = d;
}

void expr_cg_bitcast(struct expression *e)
{
    expr_cg_constant_expression(e, "bitcast");
}

void expr_cg_sign_extend(struct expression *e)
{
    expr_cg_constant_expression(e, "sext");
}

void expr_cg_zero_extend(struct expression *e)
{
    expr_cg_constant_expression(e, "zext");
}

void expr_cg_trunc(struct expression *e)
{
    expr_cg_constant_expression(e, "trunc");	
}
