#!/bin/sh
yacc -d pgrm.y
lex pgrm.l
gcc lex.yy.c y.tab.c -o xsmcompiler.exe
lex labeltranslator.l
gcc lex.yy.c -o labeltranslator.exe

