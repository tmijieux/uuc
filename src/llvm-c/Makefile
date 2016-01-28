LEX=lex
YACC=bison
CFLAGS=-Wall -ggdb -O0 -std=gnu99 -Dmalloc=gcmalloc -Dfree=gcfree \
	-Drealloc=gcrealloc -Dcalloc=gccalloc -I. `llvm-config --cflags`
LDFLAGS=-lgc -L. -lLLVM-3.5
CC=cc
SRC=	grammar.c \
	scanner.c \
	list.c \
	symbol_table.c \
	hash_table.c \
	type.c \
	symbol.c \
	error.c \
	function.c \
	expression.c \
	expr_codegen.c \
	statement.c \
	program.c \
	llvm.c \
	alloc.c \
	string.c \

OBJ=$(patsubst %.c,%.o,$(SRC))

all: ucc

ucc: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

grammar.c:grammar.y
	$(YACC) -d -o $@ --defines=grammar.tab.h $^

scanner.c: scanner.l grammar.c
	$(LEX) -o $@ $<

clean:
	$(RM) $(OBJ) grammar.c scanner.c grammar.tab.h

