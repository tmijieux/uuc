#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "statement.hpp"
#include "program.hpp"
#include "symbol.hpp"
#include "llvm.hpp"

struct statement *stmt_new(void)
{
	struct statement *stmt = (struct statement *) malloc(sizeof*stmt);
	return stmt;
}

const char *stmt_concat_list(struct list *stmt_list)
{
	int s = list_size(stmt_list);
	
	char *code, *tmp = "";
	for (int i = 1; i <= s; ++i) {
	asprintf(&code, "%s%c%s", tmp,
		 i > 1 ?'\n' : ' ',
		 ((struct statement*)list_get(stmt_list, i))->code);
	tmp = code;
	}
	return code;
}

// take a list of symbols*
const char *decl_concat_list(struct list *decl_list)
{
	int s = list_size(decl_list);
	
	char *code="", *tmp = "";
	for (int i = 1; i <= s; ++i) {
	asprintf(&code, "%s\n%s", tmp,
		 ((struct symbol*)list_get(decl_list, i))->variable.alloc_code);
	tmp = code;
	}
	
	return code;
}

const char *stmt_code_if(struct expression *cond,
			 struct statement *then)
{
	unsigned int d = prgm_get_unique_id();
	char *code;
	asprintf(&code,
		 "br label %%cond%d\n" // end previous block with a br instruction
		 "cond%d:\n"
		 "%s\n"
		 "br i1 %s, label %%then%u, label %%end%u\n"
		 "then%u:\n"
		 "%s\n"
		 "br label %%end%u\n"
		 "end%u:\n",
		 d, d,
		 cond->rvalue_code,
		 llvm_rvalue_eval(cond), d, d,
		 d,
		 then->code,
		 d,
		 d);
	
	return code;
}

const char *stmt_code_if_else(struct expression *cond,
				  struct statement *then,
				  struct statement *eelse)
{
	unsigned int d = prgm_get_unique_id();
	char *code;
	asprintf(&code,
		 "br label %%cond%d\n" // end previous block with a br instruction
		 "cond%d:\n"
		 "%s\n"
		 "br i1 %s, label %%then%u, label %%else%u\n"
		 "then%u:\n"
		 "%s\n"
		 "br label %%end%u\n"
		 "else%u:\n"
		 "%s\n"
		 "br label %%end%u\n"
		 "end%u:\n",
		 d, d,
		 cond->rvalue_code,
		 llvm_rvalue_eval(cond), d, d,
		 d,
		 then->code, d,
		 d,
		 eelse->code, d,
		 d);
	return code;
}

const char *stmt_code_for(struct expression *init,
			  struct expression *cond,
			  struct expression* next,
			  struct statement *body)
{
	unsigned int d = prgm_get_unique_id();
	char *code;
	asprintf(&code,
		 "%s\n"
		 "br label %%cond%d\n"
		 "cond%d:\n"
		 "%s\n"
		 "br i1 %s, label %%start%u, label %%end%u\n"
		 "start%u:\n"
		 "%s\n"
		 "%s\n"
		 "br label %%cond%u\n"
		 "end%u:\n",
		 init->rvalue_code,
		 d, d,
		 cond->rvalue_code,
		 llvm_rvalue_eval(cond), d, d,
		 d,
		 body->code,
		 next->rvalue_code,
		 d,
		 d);
	
	return code;
}

const char *stmt_code_while(struct expression *cond,
				struct statement *body)
{
	unsigned int d = prgm_get_unique_id();
	char *code;
	asprintf(&code,
		 "br label %%cond%d\n" // end previous block with a br instruction
		 "cond%d:\n"
		 "%s\n"
		 "br i1 %s, label %%loop%u, label %%end%u\n"
		 "loop%u:\n"
		 "%s\n"
		 "br label %%cond%u\n"
		 "end%u:\n",
		 d, d,
		 cond->rvalue_code,
		 llvm_rvalue_eval(cond), d, d,
		 d,
		 body->code,
		 d,
		 d);
	
	return code;
}

const char *stmt_code_do_while(struct expression *cond,
				   struct statement *body)
{
	unsigned int d = prgm_get_unique_id();
	char *code;
	asprintf(&code,
			 "br label %%start%d\n"
			 "start%u:\n"
			 "%s\n"
			 "%s\n"
			 "br i1 %s, label %%start%u, label %%end%u\n"
			 "end%u:\n",
			 d, d,
			 body->code,
		 cond->rvalue_code,
		 llvm_rvalue_eval(cond), d, d, d);
	
	return code;
}

const char *stmt_code_return_void(void)
{
	return "ret void";
}

const char *stmt_code_return(const struct expression *expr)
{
	char *code;
	asprintf(&code,
		 "%s\n"
		 "ret %s %s",
		 expr->rvalue_code,
		 llvm_type(expr->type),
		 llvm_rvalue_eval(expr));
	return code;
}
