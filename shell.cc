#include "command.hh"
int yyparse(void);

int main() {
	Command::_currentCommand.prompt();
	yyparse();
}
