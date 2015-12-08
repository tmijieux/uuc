#ifndef LLVM_H
#define LLVM_H

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "list.hpp"
#include "type.hpp"
#include "expression.hpp"


class LLVM {
public:
    static llvm::LLVMContext &context;
    static llvm::IRBuilder<> Builder;
    static llvm::Module *module;
};


const char *llvm_type(const struct type *t);
const char *llvm_arglist(struct list *l);
const char *llvm_arglist_type(struct list *l);

const char *llvm_rvalue_eval(const struct expression *e);
const char *llvm_lvalue_eval(const struct expression *e);

#endif //LLVM_H
