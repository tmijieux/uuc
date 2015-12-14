%{
#ifndef _GNU_SOURCE    
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>

#include <list.h>
	
#include "type.h"
#include "symbol.h"
#include "symbol_table.h"
#include "error.h"
#include "function.h"
#include "expression.h"
#include "statement.h"
#include "program.h"
#include "codegen.h"
#include "module.h"
    
extern int yylex();
struct list *declarator_list_append(struct list *l, struct symbol *s);
%}

%token <str> TOKEN_IDENTIFIER
%token <i> TOKEN_CONSTANTI
%token <f> TOKEN_CONSTANTF
%token TOKEN_MAP TOKEN_REDUCE TOKEN_EXTERN TOKEN_SIZE_OP
%token TOKEN_INC_OP TOKEN_DEC_OP TOKEN_LE_OP TOKEN_GE_OP TOKEN_EQ_OP TOKEN_NE_OP
%token TOKEN_SUB_ASSIGN TOKEN_MUL_ASSIGN TOKEN_ADD_ASSIGN
%token TOKEN_INT TOKEN_FLOAT TOKEN_VOID
%token TOKEN_IF TOKEN_ELSE TOKEN_WHILE TOKEN_RETURN TOKEN_FOR TOKEN_DO

%type <enum_type> type_name

%type <symbol> declarator parameter_declaration function_declarator prototype
%type <func> function_definition

%type <list> declarator_list declaration_list parameter_list declaration
%type <list> argument_expression_list statement_list

%type <expr> expression unary_expression comparison_expression primary_expression
%type <expr> postfix_expression multiplicative_expression additive_expression

%type <stmt> statement expression_statement compound_statement 
%type <stmt> selection_statement iteration_statement jump_statement
%type <c> assignment_operator

%start program
%union {
    // scalars:
    char *str;
    char c;
    int i;
    float f;
    enum enum_type enum_type;
    // struct 
    struct symbol *symbol;
    struct function *func;
    struct statement *stmt;
    const struct expression *expr;
    
    // utilities
    struct list *list;
};
%%
 /***************** EXPRESSION ******************/

 // expression > comparison > additive >
 //    multiplicative > unary > postfix > primary

expression                   
: unary_expression assignment_operator expression {
    const struct expression *e = NULL;
    switch ($2) {
    case '=':
	$$ = expr_assignment($1, $3);
	break;
    case '-':
	e = expr_substraction($1, $3);
	$$ = expr_assignment($1, e);
	break;
    case '+':
	e = expr_addition($1, $3);
	$$ = expr_assignment($1, e);
	break;
    case '*':
	e = expr_multiplication($1, $3);
	$$ = expr_assignment($1, e);
	break;
	
    default:
	internal_error("default clause reached, assignment operator");
	break;
    }
 }
| comparison_expression { $$ = $1; }
;

argument_expression_list
: expression  { list_append(($$ = list_new(0)), $1); }
| argument_expression_list ',' expression { list_append($$ = $1, $3); }
;

assignment_operator // type : char
: '='                { $$ = '='; }
| TOKEN_MUL_ASSIGN   { $$ = '*'; }
| TOKEN_ADD_ASSIGN   { $$ = '+'; }
| TOKEN_SUB_ASSIGN   { $$ = '-'; }
;

primary_expression
: TOKEN_IDENTIFIER  {
    struct symbol *sy = symbol_check($1);
    $$ = expr_symbol(sy);
 }
| TOKEN_CONSTANTI { $$ = expr_constant(type_int, $1); }
| TOKEN_CONSTANTF { $$ = expr_constant(type_float, $1); }
| '(' expression ')' { $$ = $2; }
| TOKEN_MAP '(' postfix_expression ',' postfix_expression ')'
  { $$ = expr_map($3, $5); }
| TOKEN_REDUCE '(' postfix_expression ',' postfix_expression ')'
  { $$ = expr_reduce($3, $5); }
| TOKEN_IDENTIFIER '(' ')'  {   // funcall
    struct symbol *sy = symbol_check($1);
    $$ = expr_funcall(sy, list_new(0));
  }
| TOKEN_IDENTIFIER '(' argument_expression_list ')' { // funcall with parameter
    struct symbol *sy = symbol_check($1);
    $$ = expr_funcall(sy, $3);
  }
| TOKEN_SIZE_OP '(' postfix_expression  ')' { $$ = expr_array_size($3);}
;

postfix_expression
: primary_expression                    { $$ = $1; }
| postfix_expression '[' expression ']' { $$ = expr_postfix($1, $3); }
;

unary_expression
: postfix_expression                 { $$ = $1; }
| TOKEN_INC_OP postfix_expression    { $$ = expr_pre_inc($2); }
| TOKEN_DEC_OP postfix_expression    { $$ = expr_pre_dec($2); }
| postfix_expression TOKEN_INC_OP    { $$ = expr_post_inc($1); }
| postfix_expression TOKEN_DEC_OP    { $$ = expr_post_dec($1); }
| '-'  unary_expression              { $$ = expr_unary_minus($2); }
;

multiplicative_expression
: unary_expression               { $$ = $1; }
| multiplicative_expression '*' unary_expression
{ $$ = expr_multiplication($1, $3); }
| multiplicative_expression '/' unary_expression
{ $$ = expr_division($1, $3); }
;

additive_expression
: multiplicative_expression             { $$ = $1; }
| additive_expression '+' multiplicative_expression
{ $$ = expr_addition($1, $3); }
| additive_expression '-' multiplicative_expression
{  $$ = expr_substraction($1, $3); }
| '(' type_name ')' additive_expression { $$ = expr_cast($4, last_type_name); }
;

comparison_expression
: additive_expression              { $$ = $1; }
| additive_expression '<' additive_expression { $$ = expr_lower($1, $3);  }
| additive_expression '>' additive_expression { $$ = expr_greater($1, $3); }
| additive_expression TOKEN_LE_OP additive_expression { $$ = expr_leq($1, $3); }
| additive_expression TOKEN_GE_OP additive_expression { $$ = expr_geq($1, $3); }
| additive_expression TOKEN_EQ_OP additive_expression { $$ = expr_eq($1, $3); }
| additive_expression TOKEN_NE_OP additive_expression { $$ = expr_neq($1, $3); }
;

// ************ DECLARATIONS *****************/

program
: external_declaration
| program external_declaration
;

// % type  ( external_declaration ) = < char * >
external_declaration
: function_definition 
| declaration {
    int si = list_size($1);
    for (int i = 1; i <= si; ++i)
	module_add_global(m, list_get($1, i));
 }
| prototype { module_add_prototype(m, $1); }
// %type < prototype > = < struct * symbol >
;

function_definition
: type_name function_declarator compound_statement {
    struct function *fun = module_get_or_create_function(m, $2);
    if ( fun_set_body(fun, $3) != 0 ) {
	fatal_error("multiple definition for function %s\n", $2->name);
    }
 }
;

prototype
: type_name function_declarator ';' {  $$ = $2; }
;

// *** declaration
type_name
: TOKEN_VOID   { $$ = TYPE_VOID; last_type_name = type_void; }
| TOKEN_INT    { $$ = TYPE_INT;  last_type_name = type_int; }
| TOKEN_FLOAT  { $$ = TYPE_FLOAT; last_type_name = type_float; }
;

// %type ( declaration ) = < struct list * < struct symbol *>  >
declaration
: type_name declarator_list ';' {
    if ($1 == TYPE_VOID) 
	error("void is not a valid type for a variable.\n");
    $$ = $2;
 }
| TOKEN_EXTERN type_name declarator_list ';' {
    if ($2 == TYPE_VOID) 
	error("void is not a valid type for a variable.\n");
    $$ = $3;
 }
;

// %type ( declaration_list ) = < struct list * < struct symbol *>  >
declaration_list
: declaration  { $$ = $1; }
| declaration_list declaration  { list_append_list($$ = $1, $2); }
;

// *** declarator  (plusieurs declarator par declarations)
// %type ( declarator ) = < struct symbol * >
declarator
: TOKEN_IDENTIFIER { $$ = symbol_new($1, last_type_name); }
| '(' declarator ')' { $$ = $2; }
| declarator '[' TOKEN_CONSTANTI ']' {
    $$ = $1;
    $$->type = type_new_array_type_reversed($1->type,
					    expr_constant(type_long, $3));
  }
| declarator '[' TOKEN_IDENTIFIER ']' {
    struct symbol *sy = symbol_check($3);
    const struct expression *expr = expr_symbol(sy);
    $$ = $1;
    $$->type = type_new_array_type_reversed($1->type, expr);
  }
| declarator '[' ']' {
    $$ = $1;
    $$->type = type_new_array_type_reversed($1->type,
					    expr_constant(type_long, 0L));
  }
;

// %type ( funtion_declarator )  = < struct symbol * >
function_declarator
: declarator '(' parameter_list ')'{ $$ = function_declare($1, $3); }
| declarator '(' ')' { $$ = function_declare($1, list_new(0)); }
;

// %type ( declarator_list ) = < struct list * < struct symbol *>  >
declarator_list
: declarator   { $$ = declarator_list_append(list_new(0), $1); }
| declarator_list ',' declarator { $$ = declarator_list_append($1, $3); }
;

// *** PARAMETER DECL

// % type ( parameter_declaration ) = < struct symbol * >
parameter_declaration
: type_name declarator {
    if ($1 == TYPE_VOID) 
	error("void is not a valid type.\n");
    $$ = $2;
    $$->symbol_type = SYM_VARIABLE;
    $$->variable.is_parameter = 1;
    $$->suffix = "param";
 }
;

// % type ( parameter_list ) = < struct list * < struct symbol * > >
parameter_list
: parameter_declaration { list_append(($$ = list_new(0)), $1); }
| parameter_list ',' parameter_declaration { list_append($$ = $1, $3); }
;

/************* STATEMENTS  **************/
statement
: compound_statement    { $$ = $1; }
| expression_statement  { $$ = $1; }
| selection_statement   { $$ = $1; }
| iteration_statement   { $$ = $1; }
| jump_statement        { $$ = $1; }
;

statement_list
: statement  { list_append(($$ = list_new(0)), $1);  }
| statement_list statement { list_append($$ = $1, $2); }
;

left_brace
: '{' {  st_push(); }
;

right_brace
: '}' { st_pop(); }
;

compound_statement
: left_brace right_brace  {
    warning("Empty block is useless\n");
    $$ = stmt_compound(NULL, NULL);
 }
| left_brace statement_list right_brace {
    $$ = stmt_compound(NULL, $2);
 }
| left_brace declaration_list right_brace {
    warning("Block with no instructions\n");
    $$ = stmt_compound(NULL, NULL); // discard the useless declaration!
 }
| left_brace declaration_list statement_list right_brace
{ $$ = stmt_compound($2, $3); }
;

expression_statement
: ';'  {  $$ = stmt_expression(NULL); }
| expression ';' {  $$ = stmt_expression($1); }
;

selection_statement
: TOKEN_IF '(' expression ')' statement {  $$ = stmt_if($3, $5); }
| TOKEN_IF '(' expression ')' statement TOKEN_ELSE statement
 { $$ = stmt_if_else($3, $5, $7);  }
| TOKEN_FOR '(' expression_statement expression_statement expression ')' statement
 { $$ = stmt_for($3->expr, $4->expr, $5, $7); }
;

iteration_statement
: TOKEN_WHILE '(' expression ')' statement {  $$ = stmt_while($3, $5); }
| TOKEN_DO statement TOKEN_WHILE '(' expression ')' ';'
{ $$ = stmt_do_while($5, $2); }
;

jump_statement
: TOKEN_RETURN ';' {  $$ = stmt_return_void(); }
| TOKEN_RETURN expression ';' {   $$ = stmt_return($2); }
;
/************************************************/

%%

struct list *declarator_list_append(struct list *l, struct symbol *s)
{
    list_append(l, s);
    s->symbol_type = SYM_VARIABLE;
    if (!st_add(s)) {
	error("symbol multiple definition: %s \n", s->name);
    }
    return l;
}
