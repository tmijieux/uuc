%{
#include <stdio.h>
#include <string.h>

#include "string2.h"
#include "type.h"
#include "symbol_table.h"
#include "scanner.h"
#include "error.h"
    
#define BUFSIZE 1024
    
static void count(void);
static int comment(void);
static void line_marker(void);

extern char* yyfilename;

#include "grammar.tab.h"
%}

D[0-9]
L[a-zA-Z_]
%option yylineno
%%
"/*"		{ comment(); }
^"#"    	{ line_marker(); }
"//".*          { count(); }
"float"		{ count(); return(TOKEN_FLOAT); }
"if"		{ count(); return(TOKEN_IF); }
"else"		{ count(); return(TOKEN_ELSE); }
"int"		{ count(); return(TOKEN_INT); }
"string"        { count(); return(TOKEN_STRING); }
"return"	{ count(); return(TOKEN_RETURN); }
"void"		{ count(); return(TOKEN_VOID); }
"while"		{ count(); return(TOKEN_WHILE); }
"for"		{ count(); return(TOKEN_FOR); }
"map"           { count(); return(TOKEN_MAP); }
"reduce"        { count(); return(TOKEN_REDUCE); }
"extern"        { count(); return(TOKEN_EXTERN); }
"do"            { count(); return(TOKEN_DO); }
"size"          { count(); return(TOKEN_SIZE_OP); }
[$]?{L}({L}|{D})*	{ count(); yylval.str=strdup(yytext);
                            return(TOKEN_IDENTIFIER); }
{D}+    	{ count(); yylval.i=atoi(yytext); return(TOKEN_CONSTANTI); }
{D}+"."{D}*     { count(); yylval.f=atof(yytext); return(TOKEN_CONSTANTF); }
L?\"(\\.|[^\\"])*\"	{ count(); yylval.str=strdup(yytext);
                          return(TOKEN_STRING_LITERAL); }
"+="		{ count(); return(TOKEN_ADD_ASSIGN); }
"-="		{ count(); return(TOKEN_SUB_ASSIGN); }
"*="		{ count(); return(TOKEN_MUL_ASSIGN); }
"++"		{ count(); return(TOKEN_INC_OP); }
"--"		{ count(); return(TOKEN_DEC_OP); }
"<="		{ count(); return(TOKEN_LE_OP); }
">="		{ count(); return(TOKEN_GE_OP); }
"=="		{ count(); return(TOKEN_EQ_OP); }
"!="		{ count(); return(TOKEN_NE_OP); }
"||"		{ count(); return(TOKEN_OR_OP); }
"&&"		{ count(); return(TOKEN_AND_OP); }
";"		{ count(); return(';'); }
"{"      	{ count(); return('{'); }
"}"      	{ count(); return('}'); }
","		{ count(); return(','); }
"/"		{ count(); return('/'); }
"="		{ count(); return('='); }
"("		{ count(); return('('); }
")"		{ count(); return(')'); }
"["     	{ count(); return('['); }
"]"     	{ count(); return(']'); }
"."		{ count(); return('.'); }
"!"		{ count(); return('!'); }
"~"		{ count(); return('~'); }
"-"		{ count(); return('-'); }
"+"		{ count(); return('+'); }
"*"		{ count(); return('*'); }
"%"		{ count(); return('%'); }
"<"		{ count(); return('<'); }
">"		{ count(); return('>'); }
[ \t\v\n\f]	{ count(); }
.		{ /* ignore bad characters */ }

%%
int yywrap()
{
	return 1;
}
/* This function reads a comment. Comments are not part of the grammar. */
static int comment(void)
{
    char c, c1;
  loop:
    while ((c = input()) != '*' && c != 0);
    if ((c1 = input()) != '/' && c != 0) {
	unput(c1);
	goto loop;
    }
    return 0;
}

int yycolno = 0;

unsigned int old_yytext_index = 0;
const char *old_yytext[YYOLDTEXT_SIZE] = {
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", ""
};


/* This function counts the number of character, for debugging purposes */
static void count(void)
{
    int i;
    for (i = 0; yytext[i] != '\0'; i++) {
	if (yytext[i] == '\n')
	    yycolno = 0;
	else if (yytext[i] == '\t')
	    yycolno += 8 - (yycolno % 8);
	else
	    yycolno++;
    }

    old_yytext_index = (old_yytext_index+1)%YYOLDTEXT_SIZE;
    old_yytext[old_yytext_index] = strstrip(yytext);
    // circular buffer for saving last tokens the parser went through
}

static void line_marker(void)
{
    char line[BUFSIZE];
    char c;
    char *filename;
    
    int i = 0;
    while ((c = input()) != '\n' && c != 0 && i < BUFSIZE) {
        line[i++] = c;
    }
    line[i] = 0;
    sscanf(line, "%d \"%ms", &yylineno, &filename);
    yyfilename = strstrip2(filename);
    free(filename);
    
//    debug("line_marker %d, %s\n", yylineno, yyfilename);
}

