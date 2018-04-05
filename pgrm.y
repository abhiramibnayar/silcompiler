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
  int rettype;
  struct ntype *type;
  struct parameterlist *parlisthead;
  int yylex(void);
  FILE *yyin;
%}
%token BEG END READ WRITE ID PLUS MINUS MUL DIV NUM ASG LT GT LE GE EQ SCON RET
%token NE WHILE DO ENDWHILE IF then ELSE ENDIF CNT BRK DECL ENDDECL INT STR MAIN
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV


%%

Program : GDeclBlock FDefBlock MainBlock
          {
            return 1;
          }
        | GDeclBlock MainBlock
                  {
                    return 1;
                  }
        ;
//assumption : function declation with same variable as function name and parameter name is allowed;
//fix lookup...use ctrl f
//use strcmp for install functions
//add codegen for nodetype 13
//add stuff for call statement

GDeclBlock : DECL GDeclList ENDDECL {printglobalsymboltable();}
           | DECL ENDDECL
           ;

GDeclList  : GDeclList GDecl
           | GDecl
           ;

GDecl : Type GIDList ';'
      ;

GIDList : GIDList ',' GID
        | GID
        ;

GID : ID {
              if(gstlookup($1->op)==NULL)
              {
              type=(struct ntype*)malloc(sizeof(struct ntype));
              type->vartype=types;
              type->arraysize=1;
              type->elemtype=NULL;
              type->width=1;
              gstinstall($1->op,type,1,NULL);
              }
              else
              {
              yyerror("redeclaration of variable");
              exit(1);
              }
              }
    | AID {
                      if(gstlookup($1->op)==NULL)
                      {
                      gstinstall($1->op,type,type->width*type->arraysize,NULL);
                      }
                      else
                      {
                      yyerror("redeclaration of variable");
                      exit(1);
                      }
          }
    | ID '(' ParamList ')' {
              if(gstlookup($1->op)==NULL)
              {
              type=(struct ntype*)malloc(sizeof(struct ntype));
              type->vartype=types;
              type->width=1;
              type->elemtype=NULL;
              type->arraysize=1;
              gstinstall($1->op,type,0,parlisthead);
              parlisthead=NULL;
              }
              else
              {
              yyerror("redeclaration of variable");
              exit(1);
              }
    }
    ;


FDefBlock : FDefBlock FDef
            { //after an entire function has been codegened.
            printlocalsymboltable();
            lsthead=NULL;
            decls=1;
            regallocate=3;
            }

          | FDef
          {
            printlocalsymboltable();

            //after an entire function has been codegened.
            lsthead=NULL;
            decls=1;
            regallocate=3;
          }
          ;

FDef : FSignature '{' LDeclBlock Body '}'
      {
        struct globalsymboltable *fgstentry=gstlookup($1->op);
        printf("\nThe return type is %d and functype is %d\n",rettype,$2->type);
       if(rettype!=$2->type)
       {
       yyerror("return type Mismatch");
       exit(1);
       }

       fprintf(fp, "L%d:\n",fgstentry->functionlabel );

      //semantic analysis of function is complete

      fprintf(fp, "PUSH BP\n");
      fprintf(fp, "MOV BP, SP\n");
      //local variables
      struct localsymboltable *localvars=lsthead;


      while(localvars!=NULL)
      {
        if(localvars->binding>0)
        fprintf(fp, "PUSH R0\n");
        localvars=localvars->nextentry;
      }
      codegen($4);

      localbinding=1;
      }



     | Type ID '(' ')' '{' LDeclBlock Body '}'
     {
     if(gstlookup($2->op)==NULL)
     {
     yyerror("Function not declared");
     exit(1);
     }
     localbinding=1;
     struct globalsymboltable *fgstentry=gstlookup($2->op);
     if(fgstentry->type->vartype!=types)
     {
       yyerror("function return type Mismatch");
       exit(1);
     }
     if(fgstentry->plist!=NULL)
     {
      yyerror("Formal and Actual Parameters Mismatch");
      exit(1);
     }
     printf("\nThe return type is %d and functype is %d\n",rettype,$2->type);
     if(rettype!=$2->type)
     {
     yyerror("return type Mismatch");
     exit(1);
     }
     fprintf(fp, "L%d:\n",fgstentry->functionlabel);
     fprintf(fp, "PUSH BP\n");
     fprintf(fp, "MOV BP, SP\n");

     $$=$7;
     reg=codegen($7);
     localbinding=1;

     }
     ;

FSignature : Type ID '(' ParamList ')'
            {

              printlocalsymboltable();
            if(gstlookup($2->op)==NULL)
            {
            yyerror("Function not declared");
            exit(1);
            }
            struct globalsymboltable *fgstentry=gstlookup($2->op);
            if(fgstentry->type->vartype!=types)
            {
              yyerror("function return type Mismatch");
              exit(1);
            }
            struct parameterlist *fparlist=fgstentry->plist;
            int count=1;
            while(fparlist!=NULL && parlisthead!=NULL)
            {
              if(strcmp(fparlist->name,parlisthead->name)==0 && fparlist->type->vartype==parlisthead->type->vartype)
              {
                if(lstlookup(fparlist->name)==NULL)
                {
                  localbinding=-count-2;
                  count++;
                  lstinstall(fparlist->name,fparlist->type);
                }
                else
                {
                  yyerror("Local Variable Parameter name conflict");
                  exit(1);
                }
                fparlist=fparlist->nextentry;
                parlisthead=parlisthead->nextentry;
                continue;
              }
              else
              {
              yyerror("Parameter Mismatch");
              exit(1);
              }
            }

            if(fparlist!=NULL || parlisthead!=NULL)
            {
              yyerror("Paramter Number Mismatch");
              exit(1);
            }
            fgstentry->size=count;
            $$=$2;
            localbinding=1;
            }
            ;

LDeclBlock : Declarations {printlocalsymboltable();decls=0;}
           ;

MainBlock : INT MAIN '(' ')' '{' LDeclBlock Body '}'
            {
            fprintf(fp, "L0:\n");
            fprintf(fp,"MOV BP,%d\n",staticmem);
            fprintf(fp, "MOV SP, BP\n");

            printlocalsymboltable();
            struct localsymboltable *localvars=lsthead;
            while((localvars!=NULL) && (localvars->binding>=0))
            {
              fprintf(fp, "PUSH R0\n");

              localvars=localvars->nextentry;

            }
            mainfunction=1;
            if(types!=1)
            {
              yyerror("Main returns integers only");
              exit(1);
            }
            reg=codegen($7);

            }
          ;


ParamList : ParamList ',' Param
          | Param
          |
          ;

Param : Type ID { //assuming that the parameter if of type int or string, not array or user defined types

                  struct parameterlist *newparlist=(struct parameterlist*)malloc(sizeof(struct parameterlist));
                  newparlist->name=(char*)malloc(sizeof(char*));
                  strcpy(newparlist->name,$2->op);
                  type=(struct ntype*)malloc(sizeof(struct ntype));
                  type->vartype=types;
                  type->arraysize=1;
                  type->width=1;
                  type->elemtype=NULL;
                  newparlist->type=type;
                  newparlist->nextentry=NULL;
                  if(parlisthead==NULL)
                  parlisthead=newparlist;
                  else
                  {
                    struct parameterlist *trv=parlisthead;
                    while(trv->nextentry!=NULL)
                    {
                      trv=trv->nextentry;
                    }
                    trv->nextentry=newparlist;
                  }

                }

      ;


Declarations : DECL DeclList ENDDECL {}

             | DECL ENDDECL {}
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

                          if(lstlookup($3->op)==NULL)
                          {
                          type=(struct ntype*)malloc(sizeof(struct ntype));
                          type->vartype=types;
                          type->arraysize=1;
                          type->elemtype=NULL;
                          type->width=1;
                          lstinstall($3->op,type);
                          }
                          else
                          {
                          yyerror("redeclaration of variable");
                          exit(1);
                          }
                          $$=$1;
                        }

          | VarList ',' AID {
                              if(lstlookup($3->op)==NULL)
                                {
                                  lstinstall($3->op,type);
                                }
                              else
                                {
                                  yyerror("redeclaration of variable");
                                  exit(1);
                                }
                              $$=$1;
                            }
        | ID {

              if(lstlookup($1->op)==NULL)
              {
              type=(struct ntype*)malloc(sizeof(struct ntype));
              type->vartype=types;
              type->arraysize=1;
              type->elemtype=NULL;
              type->width=1;
              lstinstall($1->op,type);
              }
              else
              {
              yyerror("redeclaration of variable");
              exit(1);
              }

            }
            | AID {

                      if(lstlookup($1->op)==NULL)
                      {
                      lstinstall($1->op,type);
                      }
                      else
                      {
                      yyerror("redeclaration of variable");
                      exit(1);
                      }

                    }

        ;

Body : BEG  Slist Retstmt END ';'{$$=makenode(0,0,NULL,5,$2,$3,NULL);decls=1;}
     | BEG Retstmt END ';' {$$=$2;decls=1;}
     ;

Retstmt : RET ID ';'  {rettype=$2->type;$$=makenode(0,0,NULL,12,$2,NULL,NULL);}
        | RET NUM ';' {rettype=1;$$=makenode(0,0,NULL,12,$2,NULL,NULL);}
        | RET SCON ';' {rettype=3;$$=makenode(0,0,NULL,12,$2,NULL,NULL);}
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
  | ID '(' ')' {$$=makenode(0,0,$1->op,13,NULL,NULL,NULL);}
  | ID '(' ArgList ')' {$$=makenode(0,0,$1->op,13,$3,NULL,NULL);}
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

ArgList : ArgList ',' E {$$=makenode(0,$3->type,NULL,14,$1,$3,NULL);}
        | E {$$=$1;}
        ;

L : L '['E']' {
               type=type->elemtype;
               $$=makenode(type->width,$1->type,$1->op,11,$1,$3,NULL);

              }
  | ID '['E']' {
                if(lstlookup($1->op)==NULL && gstlookup($1->op)==NULL)
                {
                  yyerror("Array not declared");
                  exit(1);
                }
                if(lstlookup($1->op)!=NULL)
                type=lstlookup($1->op)->type;
                else
                type=gstlookup($1->op)->type;
                $$=makenode(type->width,$1->type,$1->op,11,$1,$3,NULL);

                }
  ;

AID : ID ARR {
            if(decls==0)
            {
            struct ntype *newtype;
            if(lstlookup($1->op)==NULL && gstlookup($1->op)==NULL)
            {
              yyerror("Array not declared");
              exit(1);
            }
            if(lstlookup($1->op)!=NULL)
            newtype=lstlookup($1->op)->type;
            else
            newtype=gstlookup($1->op)->type;
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
		FILE *fpr = fopen(argv[1], "r");
		if(fpr)
			yyin = fpr;
	}
  fp=fopen("pgrm.xsm","w");
  fprintf(fp,"0\nL0\n0\n0\n0\n0\n0\n0\n");


yyparse();
return 0;
}
