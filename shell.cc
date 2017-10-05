#include "command.hh"
#include "signal.h"
#include "stdlib.h"

int yyparse(void);

extern "C" void ctrl-c (int sig) {
	printf("\n");
	Command::_currentCommand.prompt();
}

int main() {
	struct sigaction sigA1;
	sigA1.sa_handler = ctrl-c;
	sigemptyset(&sigA1.sa_mask);
	sigA1.sa_flags = SA_RESTART;
	int error = sigaction(SIGINT, &sigA1, NULL);
	if(error){
		perror("sigaction");
		exit(-1);
	}

	Command::_currentCommand.prompt();
	yyparse();
}
