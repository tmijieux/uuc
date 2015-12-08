#ifndef FUNCTION_H
#define FUNCTION_H

#include "symbol.hpp"

struct function {
	struct symbol *symb;
	const char *code;
};

void fun_init(void);
void fun_exit(void);
int fun_defined(struct symbol *symb);
void fun_define(struct symbol *symb);
struct function * fun_new(struct symbol *sy, const char *code);

#endif //FUNCTION_H
