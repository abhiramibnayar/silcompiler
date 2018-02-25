#!/bin/sh
yacc -d pgrm.y
lex pgrm.l
gcc lex.yy.c y.tab.c -o comp1.exe
lex labeltranslator.l
gcc lex.yy.c

