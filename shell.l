
/*
 *
 * CS-252 Fall 2017
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <unistd.h>
#include "y.tab.hh"
#include "command.hh"

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

["][^\n\"]*["] { 
	/* quotes */
	yylval.string_val = strdup(yytext+1);

    if (yylval.string_val[yyleng-2] == '"')
		yylval.string_val[yyleng-2] = 0;
	return WORD;
} 

[^ \t\n|><&]*\\[^ \t\n]* {
	/* escaping */
//printf("yyval=%s\n",yylval.string_val);
	int i = 0;
	char * escape = (char *)malloc(200);
	char * temp = yytext;

	while(*temp){
		if (*temp == '\\'){
			if(*(temp+1) == '\\'){
				temp = temp + 2;
				escape[i++] = '\\';
			} else {
				escape[i++] = *(++temp);
			} 
		} else {
			escape[i++] = *temp;
		}
		temp++;
	}

	escape[i] = '\0';
	yylval.string_val = strdup(escape);
	return WORD;
}

[^ \t\n][^ \t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.string_val = strdup(yytext);
  return WORD;
}
