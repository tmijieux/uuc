#ifndef STATEMENT_H
#define STATEMENT_H

#include "list.hpp"
#include "expression.hpp"

enum statement_type {
	STMT_COMPOUND,
	STMT_EXPR,
	STMT_SELECTION,
	STMT_ITERATION
};

struct statement {
	enum statement_type statement_type;
	struct expression *expr;
	const char *code;
};

struct statement *stmt_new(void);
const char *stmt_concat_list(struct list *stmt_list);
const char *decl_concat_list(struct list *decl_list);

const char *stmt_code_if(struct expression *cond,
			 struct statement *then);

const char *stmt_code_if_else(struct expression *cond,
				  struct statement *then,
				  struct statement *eelse);

const char *stmt_code_for(struct expression *init,
			  struct expression *cond,
			  struct expression* next,
			  struct statement *body);

const char *stmt_code_while(struct expression *cond,
				struct statement *body);

const char *stmt_code_do_while(struct expression *cond,
				   struct statement *body);

const char *stmt_code_return_void(void);
const char *stmt_code_return(const struct expression *expr);

#endif //STATEMENT_H
