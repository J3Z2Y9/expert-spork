%{
#include"tokens.h"
%}

%%

"int"                  { return TOK_INT; }
"return"               { return TOK_RETURN; }
"main"                 { return TOK_MAIN;}

[a-zA-Z_][a-zA-Z0-9_]* { return TOK_IDENTIFIER; }
[0-9]+                 { return TOK_INTEGER; }

"*"                    { return TOK_MULTIPLY; }
"/"                    { return TOK_DIVIDE; }
"%"                    { return TOK_MOD;}
"+"                    { return TOK_PLUS; }
"-"                    { return TOK_MINUS; }



"="                    { return TOK_ASSIGN; }
"=="                   { return TOK_EQUAL; }
"!="                   { return TOK_NOT_EQUAL; }
"<"                    { return TOK_LESS; }
"<="                   { return TOK_LESS_EQUAL; }
">"                    { return TOK_GREATER; }
">="                   { return TOK_GREATER_EQUAL; }

"&"                    { return TOK_AND;}
"^"                    { return TOK_XOR;}
"|"                    { return TOK_OR;}


"("                    { return TOK_LPAREN; }
")"                    { return TOK_RPAREN; }
"{"                    { return TOK_LBRACE;}
"}"                    { return TOK_RBRACE;}  
";"                    { return TOK_SEMICOLON; }
","                    {return TOK_COMMA;}


[ \t]+                { /* ºöÂÔ¿Õ°××Ö·û */ }
"\n"                   { /* ¸ú×ÙÐÐºÅ */ }
.                      { printf("Unexpected character: %c\n", *yytext); }

%%

int yywrap() {
    return 1;
}

