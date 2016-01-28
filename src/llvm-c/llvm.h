#ifndef LLVM_H
#define LLVM_H

#include "list.h"
#include "type.h"
#include "expression.h"

const char *llvm_type(const struct type *t);
const char *llvm_arglist(struct list *l);
const char *llvm_arglist_type(struct list *l);

const char *llvm_rvalue_eval(const struct expression *e);
const char *llvm_lvalue_eval(const struct expression *e);

#endif //LLVM_H
