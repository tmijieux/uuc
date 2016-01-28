#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "symbol.h"
#include "type.h"
#include <llvm-c/Core.h>

enum expression_type {
    // unary
    EXPR_SYMBOL, // 
    
    EXPR_CONSTANT,
    EXPR_UNARY_MINUS,  // -x

    EXPR_PRE_INC,  //  ++x
    EXPR_PRE_DEC,  //   --x
    EXPR_POST_INC,  //  x++
    EXPR_POST_DEC,  //  x--
    
    EXPR_POSTFIX, // a[i]
    
    // binary
    EXPR_MULTIPLICATION,
    EXPR_DIVISION,
    
    EXPR_ADDITION,
    EXPR_SUBSTRACTION,

    EXPR_LOWER,
    EXPR_GREATER,
    EXPR_LEQ,
    EXPR_GEQ,
    EXPR_EQ,
    EXPR_NEQ,
    EXPR_ASSIGNMENT,
    
    EXPR_MAP,
    EXPR_REDUCE,
    EXPR_FUNCALL,
    EXPR_FUNCALL_PARAMS
    
};

struct expression {
    enum expression_type expression_type;
    const struct type *type;

    union {
	struct {
	    struct expression *operand;
	};
	struct {
	    struct expression *operand1;
	    struct expression *operand2;
	};
	struct list *args; // expression list
    };

    int constant;
    union {
	int constanti;
	float constantf;
	struct symbol *symbol;
    };

    char *source_code;
    
    LLVMValueRef value;
};

struct expression *expr_new(enum expression_type, ...);

#endif //EXPRESSION_H
