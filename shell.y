
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
void expandWildCards(char * prefix, char * arg);
int cmpfunc(const void * file1, const void * file2);
bool is_dir(const char * path);

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
  | NEWLINE { Command::_currentCommand.prompt();}
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
	  if(strcmp(Command::_currentSimpleCommand->_arguments[0], "echo") == 0 && strchr($1, '?'))
      	Command::_currentSimpleCommand->insertArgument( $1 );
	  else
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


int cmpfunc (const void *file1, const void *file2) {
	const char *_file1 = *(const char **)file1;
	const char *_file2 = *(const char **)file2;
	return strcmp(_file1, _file2);
}

void expandWildCards(char * prefix, char * arg) {

	char * temp = arg;
	char * save = (char *) malloc (strlen(arg) + 10);
	char * dir = save;

	if (temp[0] == '/') *(save++) = *(temp++);

	while (*temp != '/' && *temp) *(save++) = *(temp++);
	*save = '\0';

	if (strchr(dir, '*') || strchr(dir, '?')) {
		if (!prefix && arg[0] == '/') {
			prefix = strdup("/");
			dir++;
		}  

		char * reg = (char *) malloc (2*strlen(arg) + 10);
		char * a = dir;
		char * r = reg;

		*(r++) = '^';
		while (*a) {
			if (*a == '*') { *(r++) = '.'; *(r++) = '*'; }
			else if (*a == '?') { *(r++) = '.'; }
			else if (*a == '.') { *(r++) = '\\'; *(r++) = '.'; }
			else { *(r++) = *a; }
			a++;
		}
		*(r++) = '$'; *r = '\0';

		regex_t re;

		int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);

		char * toOpen = strdup((prefix)?prefix:".");
		DIR * dir = opendir(toOpen);
		if (dir == NULL) {
			perror("opendir");
			return;
		}

		struct dirent * ent;
		regmatch_t match;

		while ((ent = readdir(dir)) != NULL) {
			if (!regexec(&re, ent->d_name, 1, &match, 0)) {
				if (*temp) {
					if (ent->d_type == DT_DIR) {
						char * nPrefix = (char *) malloc (150);
						if (!strcmp(toOpen, ".")) nPrefix = strdup(ent->d_name);
						else if (!strcmp(toOpen, "/")) sprintf(nPrefix, "%s%s", toOpen, ent->d_name);
						else sprintf(nPrefix, "%s/%s", toOpen, ent->d_name);
						expandWildCards(nPrefix, (*temp == '/')?++temp:temp);
					}
				} else {
					
					if (nEntries == maxEntries) { maxEntries *= 2; entries = (char **) realloc (entries, maxEntries * sizeof(char *)); }
					char * argument = (char *) malloc (100);
					argument[0] = '\0';
					if (prefix) sprintf(argument, "%s/%s", prefix, ent->d_name);

					if (ent->d_name[0] == '.') {
						if (arg[0] == '.') {
							entries[nEntries++] = (argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
						}
					} else {
						entries[nEntries++] = (argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
					}
				}
			}
		}
		closedir(dir);
	} else {
		char * preToSend = (char *) malloc (100);
		if (prefix) sprintf(preToSend, "%s/%s", prefix, dir);
		else preToSend = strdup(dir);

		if (*temp) expandWildCards(preToSend, ++temp);
	}
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
