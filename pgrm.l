%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "y.tab.h"
  int num;

%}

%%

[0-9]+ {
        num=atoi(yytext);yylval=makenode(num,1,NULL,1,NULL,NULL,NULL);return NUM;
        }
"begin" {return BEG;}
"read" {return READ;}
"write" {return WRITE;}
"end" {return END;}
"if" {return IF;}
"else" {return ELSE;}
"do" {return DO;}
"endwhile" {return ENDWHILE;}
"then" {return then;}
"endif" {return ENDIF;}
"while" {return WHILE;}
"break" {return BRK;}
"continue" {return CNT;}
"decl" {return DECL;}
"enddecl" {return ENDDECL;}
"int" {return INT;}
"str" {return STR;}
["][a-z0-9 ]+["] {yylval=makenode(0,3,yytext,10,NULL,NULL,NULL);return SCON;}
[a-z0-9]+ {yylval=makenode(0,1,yytext,2,NULL,NULL,NULL);return ID;}
"[" {return *yytext;}
"]" {return *yytext;}

"=" {return ASG;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return MUL;}
"/" {return DIV;}
"==" {return EQ;}
"<" {return LT;}
">" {return GT;}
"<=" {return LE;}
">=" {return GE;}
"!=" {return NE;}
[\t] {}
[()] {return *yytext;}
[\n] {}
[ ] {}
"," {return *yytext;}
";" {return *yytext;}
. {printf("%s",yytext);yyerror("Unknown character found ");exit(1);}

%%
int yywrap(void){
return 1;
}
