
/*
 *
 * CS-252 Fall 2017
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

">>" {
	return GREATGREAT;
}

"2>" {
	return TWOGREAT;
}

"|" {
	return PIPE;
}

"&" {
	return AMPERSAND;
}

"<" {
	return LESS;
}

">&" {
	return GREATAMPERSAND;
}

">>&" {
	return GREATGREATAMPERSAND;
}

[^ \t\n][^ \t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.string_val = strdup(yytext);
  return WORD;
}

["][^\n\"]*["] { 
	/* quotes */
	++yytext;
	yytext[strlen(yytext)] = '\0';
	yylval.string_val = strdup(yytext);
	return WORD;
} 
