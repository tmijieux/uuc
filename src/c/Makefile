LEX=lex
YACC=bison
CFLAGS=-Wall -ggdb -O0 -std=gnu99 -I. -I./util 
LDFLAGS=-L. -lutil
CC=cc
SRC=	grammar.c \
	scanner.c \
	symbol_table.c \
	type.c \
	symbol.c \
	error.c \
	function.c \
	expression.c \
	statement.c \
	program.c \
	main.c \
	module.c \
	expr_codegen.c \
	stmt_codegen.c \
	symb_codegen.c \
	type_codegen.c \
	libutil.a \

ifndef NOGC
CFLAGS+=-Dmalloc=gcmalloc -Dfree=gcfree \
	-Drealloc=gcrealloc -Dcalloc=gccalloc
LDFLAGS+=-lgc
endif

all:

OBJ=$(patsubst %.c,%.o,$(SRC))

all: uuc

uuc: $(OBJ) libutil.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

grammar.c:grammar.y
	$(YACC) -d -o $@ --defines=grammar.tab.h $^

scanner.c: scanner.l grammar.c
	$(LEX) -o $@ $<

libutil.a:  util
	make -C util/
	cp  util/libutil.a libutil.a

clean:
	$(RM) $(OBJ) grammar.c scanner.c grammar.tab.h *~
	make -C util/ clean

fullclean: clean
	$(RM) uuc -f

edit: 
	emacs -mm *.c *.h *.y *.l &
