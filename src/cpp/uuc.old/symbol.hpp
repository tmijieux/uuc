#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include <stdint.h>
#include "list.hpp"
#include "type.hpp"
#include "llvm.hpp"

#define SYM_MAX_SIZE 16

enum symbol_type {
    SYM_VARIABLE,
    SYM_FUNCTION		// no storage, always resolved at compile/link time
};

enum variable_storage {
    VAR_STATIC,
    VAR_AUTO
};

enum visibility {
    VSB_DEFAULT,
    VSB_HIDDEN,
    VSB_PROTECTED
};

enum linkage {
    LNK_PRIVATE,
    LNK_INTERNAL,
    LNK_AVAILABLE_EXTERNALLY,
    LNK_LINKONCE,
    LNK_WEAK,
    LNK_COMMON,
    LNK_APPENDING,
    LNK_EXTERN_WEAK,
    LNK_LINKONCE_ODR,
    LNK_WEAK_ODR,
    EXTERNAL
};

struct symbol_variable {
    int is_global;
    int is_parameter;

    int last_assignment_ssa;
    const char *alloc_code;
	
    int assigned_constant;
    enum enum_type constant_type;
    union {
	float valuef;
	float valuei;
    };
};

struct symbol_function {
	
};
	
struct symbol {
    const char *name;

    enum symbol_type symbol_type;
    const struct type *type;

    enum linkage linkage;
    enum visibility visibility;
	
    size_t size;
    union {
	struct symbol_variable variable;
	struct symbol_function function;
    };

    llvm::Value *value;
};

struct symbol *symbol_new(const char *name, const struct type *t);
void symbol_print(const struct symbol *sy);
struct symbol * symbol_check(const char *name);

#endif //SYMBOL_H
