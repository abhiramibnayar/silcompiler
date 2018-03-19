%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #define YYSTYPE tnode *
  #include "pgrm.h"
  #include "pgrm.c"
  int reg;
  char *op = "abcdefg";
  int types;
  struct ntype *type;
  int yylex(void);
  FILE *yyin;
%}
%token BEG END READ WRITE ID PLUS MINUS MUL DIV NUM ASG LT GT LE GE EQ SCON
%token NE WHILE DO ENDWHILE IF then ELSE ENDIF CNT BRK DECL ENDDECL INT STR
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV


%%
program : BEG Declarations Slist END {
                          $$=$3;
                          printsymtable();
                          fp=fopen("pgrm.xsm","w");
                          fprintf(fp,"0\n2056\n0\n0\n0\n0\n0\n0\n");
                          fprintf(fp,"MOV SP,%d\n",staticmem);
                          reg=codegen($$);

                          fprintf(fp,"MOV R2,\"Exit\"\nPUSH R2\nPUSH R2\n");
                          fprintf(fp,"PUSH R2\nPUSH R2\nPUSH R2\nCALL 0\nRET");
                          fclose(fp);
                          exit(1);
                          }

        | BEG Declarations END {$$=NULL;exit(1);}
        | BEG END {$$=NULL;exit(1);}
        ;
        Declarations : DECL DeclList ENDDECL {decls=0;}

             | DECL ENDDECL {decls=0;}
             ;

DeclList : DeclList Decl
         | Decl
         ;
Decl : Type VarList ';'
     ;

Type : INT {types=1;}
     | STR {types=3;}
     ;

VarList : VarList ',' ID {

                          if(lookup($3->op)==NULL)
                          {
                          type=(struct ntype*)malloc(sizeof(struct ntype));
                          type->vartype=types;
                          type->arraysize=1;
                          type->elemtype=NULL;
                          type->width=1;
                          install($3->op,type);
                          }
                          else
                          {
                          yyerror("redeclaration of variable");
                          exit(1);
                          }
                          $$=$1;
                        }
        | VarList ',' AID {
                            if(lookup($3->op)==NULL)
                              {

                                install($3->op,type);

                              }
                              else
                              {
                              yyerror("redeclaration of variable");
                              exit(1);
                              }
                              $$=$1;

                              }

        | ID {

              if(lookup($1->op)==NULL)
              {
              type=(struct ntype*)malloc(sizeof(struct ntype));
              type->vartype=types;
              type->arraysize=1;
              type->elemtype=NULL;
              type->width=1;
              install($1->op,type);
              }
              else
              {
              yyerror("redeclaration of variable");
              exit(1);
              }

            }
        | AID {

                  if(lookup($1->op)==NULL)
                  {
                  install($1->op,type);
                  }
                  else
                  {
                  yyerror("redeclaration of variable");
                  exit(1);
                  }

                }
        ;

Slist : Slist stmt {$$=makenode(0,0,NULL,5,$1,$2,NULL);}
      | stmt {$$=$1;}
      ;
stmt : inputstmt {$$=$1;}
     | outputstmt {$$=$1;}
     | asgstmt{$$=$1;}
     | ifstmt{$$=$1;}
     | whilestmt{$$=$1;}
     | brkstmt{$$=$1;}
     | cntstmt {$$=$1;}
     ;

ifstmt : IF '(' E ')' then Slist ELSE Slist ENDIF ';' {op="if";$$=makenode(0,0,op,7,$3,$8,$6);}
       | IF '(' E ')' then Slist ENDIF ';' {op="if";$$=makenode(0,0,op,7,$3,$6,NULL);}
       ;

whilestmt : WHILE '(' E ')' DO Slist ENDWHILE';' {op="while";$$=makenode(0,0,op,7,$3,$6,NULL);}
               ;
inputstmt : READ '(' ID ')' ';' {op="read";$$=makenode(0,0,op,3,$3,NULL,NULL);}
          | READ '(' AID ')' ';' {op="read";$$=makenode(0,0,op,3,$3,NULL,NULL);}
          | READ '(' L ')' ';' {op="read";$$=makenode(0,0,op,3,$3,NULL,NULL);}
          ;
outputstmt : WRITE '(' E ')' ';' {op="write";$$=makenode(0,0,op,4,$3,NULL,NULL);}
          ;
asgstmt : ID ASG E';' {op="=";$$=makenode(0,0,op,6,$1,$3,NULL);}
        |  AID ASG E';' {op="=";$$=makenode(0,0,op,6,$1,$3,NULL);}
        |  L ASG E';' {op="=";$$=makenode(0,0,op,6,$1,$3,NULL);}
        ;
brkstmt : BRK';'{op="break",$$=makenode(0,0,op,9,NULL,NULL,NULL);}
        ;
cntstmt : CNT';'{op="continue",$$=makenode(0,0,op,9,NULL,NULL,NULL);}

E : ID {$$=$1;}
  | AID {$$=$1;}
  | L {$$=$1;}
  | E LT E {op="<";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E GT E {op=">";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E LE E {op="<=";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E GE E {op=">=";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E NE E {op="!=";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E EQ E {op="==";$$=makenode(0,2,op,8,$1,$3,NULL);}
  | E PLUS E {op="+";$$=makenode(0,1,op,6,$1,$3,NULL);}
  | E MINUS E {op="-";$$=makenode(0,1,op,6,$1,$3,NULL);}
  | E MUL E {op="*";$$=makenode(0,1,op,6,$1,$3,NULL);}
  | E DIV E {op="/";$$=makenode(0,1,op,6,$1,$3,NULL);}
  | '(' E ')' {$$=$2;}
  | NUM {$$=$1;}
  | SCON {$$=$1;}
  ;

L : L '['E']' {
               type=type->elemtype;
               $$=makenode(type->width,$1->type,$1->op,11,$1,$3,NULL);

              }
  | ID '['E']' {
                type=lookup($1->op)->type;
                $$=makenode(type->width,$1->type,$1->op,11,$1,$3,NULL);

                }
  ;

AID : ID ARR {
            if(decls==0)
            {
            struct ntype *newtype=lookup($1->op)->type;
            struct ntype *ttype=type;
            int s=0;
            while(newtype!=NULL)
            {
            s=s+ttype->arraysize*newtype->width;
            newtype=newtype->elemtype;
            ttype=ttype->elemtype;
            }
            $1->val=s;
            }
            $$=$1;
}

ARR : '['NUM']' ARR {
                    struct ntype *newtype=(struct ntype*)malloc(sizeof(struct ntype));
                    newtype->vartype=types;
                    newtype->arraysize=$2->val;
                    newtype->width=type->width*type->arraysize;
                    newtype->elemtype=type;

                    type=newtype;
                    }
    | '['NUM']' {
                  struct ntype *newtype=(struct ntype*)malloc(sizeof(struct ntype));
                  newtype->vartype=types;
                  newtype->arraysize=$2->val;
                  newtype->width=1;
                  newtype->elemtype=NULL;
                  type=newtype;
                }
    ;


%%

yyerror(char const *s)
{
  printf("yyerror %s\n",s);

}

int main(int argc, char* argv[])
{
if(argc > 1)
	{
		FILE *fp = fopen(argv[1], "r");
		if(fp)
			yyin = fp;
	}
yyparse();
return 0;
}
