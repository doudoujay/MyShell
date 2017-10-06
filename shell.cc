#include "command.hh"
#include "signal.h"
#include "stdlib.h"
#include "stdio.h"

int yyparse(void);

extern "C" void ctrlC (int sig) {
	printf("\nctrl c works\n");
	Command::_currentCommand.prompt();
}

int main() {
	struct sigaction sigA1;
	sigA1.sa_handler = ctrlC;
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
