#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <assert.h>

#include "llvm.h"
#include "symbol.h"
#include "type.h"
#include "list.h"
#include "expression.h"

struct llvm {
    LLVMBuilderRef builder;
    LLVMModuleRef module;
    
};
