#ifndef CODEGEN_H
#define CODEGEN_H

#include "list.h"
#include "type.h"
#include "expression.h"

const char *type_cg(const struct type *t);
const char *type_cg_arglist(const struct list *l); // list of symbols
const char *type_cg_arglist_nameless(const struct list *l);
	
const char *expr_cg_rvalue_eval(const struct expression *e);
const char *expr_cg_lvalue_eval(const struct expression *e);

void symb_cg(struct symbol *sy);
void expr_cg(const struct expression * e);

#endif //CODEGEN_H
