%{
	#include<stdio.h>
	#include<string.h>
	#include<stdlib.h>
	int labtab[100];
	int ind=0;
  int ln=0;
	int flag=0;
	char *strn;
	int num;
	FILE *fp,*fp1;
%}

%%
[L][0-9]+":"[\n] {strn=(char*)malloc(sizeof(char*));
									strncpy(strn,yytext+1,yyleng-3);

									num=atoi(strn);
									labtab[num]= 2056 + 2*(ln-8);
									ind++;
									}
[L][0-9]+ {
						if(flag==0){

							fprintf(fp1,"%s",yytext);
						}
						if(flag==1)
						{
						strn=(char*)malloc(sizeof(char*));

						strcpy(strn,yytext+1);

						num=atoi(strn);

						fprintf(fp1,"%d",labtab[num]);
						}
						}
[\n] {ln++;fprintf(fp1,"%s",yytext);}
. {fprintf(fp1,"%s",yytext);}
%%

int yywrap()
{
if(flag==0)
{
	fclose(fp);
	fclose(fp1);

	fp=fopen("outl.xsm","r");

	if(fp)
	yyin=fp;

	fp1=fopen("outl1.xsm","w");
	flag=1;
	return 0;
}
else
{
	fclose(fp);
	fclose(fp1);
	return 1;
}
	return 1;
}

int main(int argc, char* argv[])
{
	if(argc > 1)
	{
		fp1=fopen("outl.xsm","w");
		fp=fopen(argv[1], "r");
		if(fp)
			yyin = fp;
	}
	yylex();
	return 1;
}
