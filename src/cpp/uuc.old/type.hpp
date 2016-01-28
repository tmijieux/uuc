#ifndef TYPE_H
#define TYPE_H

#include <stdlib.h>
#include <stdint.h>

struct list;

#define  SIZE_INT	 sizeof(int)
#define  SIZE_FLOAT   sizeof(float)
#define  SIZE_ADDR	sizeof(void*)
	
enum enum_type {
	TYPE_UNDEF,
	TYPE_GENERIC, // generic is to allow to continue
	// parsing when an unresolved symbol is encountered
	
	TYPE_VOID,	// cannot be applied on a variable
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_ARRAY,
	TYPE_FUNCTION // cannot be applied on a variable
};

   
struct type_array {
	size_t array_size;
	const struct type *values;
};

struct type_function {
	const struct type *return_value;
	
	uint16_t argc;
	struct list *args;
};

/**
 * type variables are designed to be immutable
 * consider using type_new to create instance for TYPE_ARRAY and TYPE_FUNCTION
 * but use the type_int, type_float, type_void, and type_generic,
 * and type_undef methods for the basic type so the instance can be shared
 *
 */
struct type {
	enum enum_type type;
	
	union {
	struct type_array array_type;
	struct type_function function_type;
	};
	size_t size;
};

const struct type *
type_new_function_type(const struct type* return_value,  struct list *args);

const struct type *
type_new_array_type(const struct type* values, size_t array_size);


int type_equal(const struct type *t1, const struct type *t2);
void type_print(const struct type *t, int level);
void type_print_line(const struct type *t);

extern const struct type *type_undef;
extern const struct type *type_generic;
extern const struct type *type_int;
extern const struct type *type_float;
extern const struct type *type_void;
	
/**
 *  This variable is used to remember the last type_name that was read
 *  to allow the program to know which type_name it should apply when
 *  it encounters a declarator
 */
extern const struct type *last_type_name;

#endif //TYPE_H
