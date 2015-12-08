#ifndef _GNU_SOURCE    
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "color.h"
#include "hash_table.h"

static struct hash_table *colors;
static struct stack *clr_stack;

int COLOR_LEN;

#define RESET "\e[39m"

#define ADD_COLOR(x, y)				\
    ht_add_entry(colors, x , y)


__attribute__((constructor))
static void clr_init(void)
{
    colors = ht_create(100, NULL);

    ADD_COLOR( "RESET",      RESET );
    ADD_COLOR( "red",        "\e[91m" );
    ADD_COLOR( "light blue", "\e[96m" );
    ADD_COLOR( "fushia",     "\e[95m" );
    ADD_COLOR( "green",      "\e[92m" );
    ADD_COLOR( "yellow",     "\e[93m" );

    COLOR_LEN = strlen(color("green", ""));

    clr_stack = stack_create(200);
    stack_push(clr_stack, RESET);
}

const char *color(const char *clr, const char *message)
{
    char *clr_code = "";
    if ( ht_get_entry(colors, clr, &clr_code) != 0 )
	{
	    fprintf(stderr, "Color Error: color %s doesn't exist\n", clr);
	    return message;
	}

    char *clr_message;
    asprintf(&clr_message, "%s%s"RESET, clr_code, message);
    return clr_message;
}


void clr_output_push(FILE *output, const char *clr)
{
    char *clr_code = "";
    if ( ht_get_entry(colors, clr, &clr_code) != 0 )
	{
	    fprintf(stderr, "Color Error: color %s doesn't exist\n", clr);
	    return ;
	}

    fprintf(output, "%s", clr_code);
    stack_push(clr_stack, clr_code);
    // push color on stack
}

void clr_output_pop(FILE *output)
{
    if ( stack_size( clr_stack ) > 1 ) 
	stack_pop(clr_stack);
    
    fprintf(output, "%s", (const char *) stack_peek(clr_stack));
}
