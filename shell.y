
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <string_val> WORD
%token NOTOKEN GREAT NEWLINE PIPE GREATGREAT AMPERSAND LESS GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "command.hh"

void yyerror(const char * s);
int yylex();

void expandWildCardsIfNecessary(char * arg);
%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
   pipe_list iomodifier_list background_optional NEWLINE { 
//printf("   Yacc: Execute command\n");
    Command::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Command::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
//printf("   Yacc: insert argument \"%s\"\n", $1);
    //Command::_currentSimpleCommand->insertArgument( $1 );
	expandWildCardsIfNecessary($1);
  }
  ;

command_word:
  WORD {
//printf("   Yacc: insert command \"%s\"\n", $1);
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

pipe_list:
	pipe_list PIPE command_and_args
	| command_and_args
	;

iomodifier_opt:
  	GREAT WORD {
//printf("   Yacc: insert output \"%s\"\n", $2);
    	Command::_currentCommand._outFile = strdup($2);
		Command::_currentCommand._outCounter++;
  	}
	| GREATGREAT WORD {
//printf("   GREATGREAT WORD: insert output \"%s\"\n", $2);
    	Command::_currentCommand._outFile = strdup($2);
    	Command::_currentCommand._append = 1;
		Command::_currentCommand._outCounter++;
	}
	| GREATAMPERSAND WORD {
//printf("   Yacc: insert output \"%s\"\n", $2);
    	Command::_currentCommand._outFile = strdup($2);
    	Command::_currentCommand._errFile = strdup($2);
		Command::_currentCommand._outCounter++;
	}
	| GREATGREATAMPERSAND WORD {
//printf("   Yacc: insert output \"%s\"\n", $2);
    	Command::_currentCommand._outFile = strdup($2);
    	Command::_currentCommand._errFile = strdup($2);
    	Command::_currentCommand._append = 1;
		Command::_currentCommand._outCounter++;
	}
	| LESS WORD {
//printf("   Yacc: insert input \"%s\"\n", $2);
    	Command::_currentCommand._inFile = strdup($2);
		Command::_currentCommand._inCounter++;
	}
	| TWOGREAT WORD {
//printf("   Yacc: insert output \"%s\"\n", $2);
    	Command::_currentCommand._errFile = strdup($2);
	}
  	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	| iomodifier_opt
	| /* can be empty */
	;

background_optional:
	AMPERSAND {
		Command::_currentCommand._background = 1;
	}
	| /* can be empty */
	;
	
%%

int maxEntries = 20;
int nEntries = 0;
char ** entries;

void expandWildCardsIfNecessary(char * arg) {

	maxEntries = 20;
	nEntries = 0;
	entries = (char **) malloc (maxEntries * sizeof(char *));

	if (strchr(arg, '*') || strchr(arg, '?')) {
		expandWildCards(NULL, arg);
		qsort(entries, nEntries, sizeof(char *), cmpfunc);
		for (int i = 0; i < nEntries; i++) Command::_currentSimpleCommand->insertArgument(entries[i]);
	}
	else {
		Command::_currentSimpleCommand->insertArgument(arg);
	}
	return;
}

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
