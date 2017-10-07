#include "command.hh"
#include "signal.h"
#include "stdlib.h"
#include "stdio.h"

int yyparse(void);

extern "C" void ctrlC (int sig) {
	printf("\nctrl c works\n");
	Command::_currentCommand.prompt();
}

extern "C" void zombie(int sig) {
	int pid = wait3(0, 0, NULL);
	while (waitpid(-1, NULL, WNOHANG) > 0);
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

	//Zombie Process
	struct sigaction sigA2;
	sigA2.sa_handler = zombie;
	sigemptyset(&sigA2.sa_mask);
	sigA2.sa_flags = SA_RESTART;
	error =  sigaction(SIGCHLD, &sigA2, NULL);
	if (error) {
		perror("sigaction");
		exit(-1);
	}

	Command::_currentCommand.prompt();
	yyparse();
}
