/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include "command.hh"

extern char **environ; //Holds enviroment variables
bool onError = false;

using namespace std;

Command::Command()
{
	// Create available space for one simple command
	_numOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_append = 0;
	_background = 0;
	_inCounter = 0;
	_outCounter = 0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
	if ( _numOfAvailableSimpleCommands == _numOfSimpleCommands ) {
		_numOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numOfSimpleCommands ] = simpleCommand;
	_numOfSimpleCommands++;
}

void Command:: clear() {
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inFile ) {
		free( _inFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_inCounter = 0;
	_outCounter = 0;
	_append = 0;
}

void Command::print() {
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inFile?_inFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

int Command::BuiltIn(int i) {
	if(strcmp(_simpleCommands[i]->_arguments[0], "setenv") == 0){
		int error = setenv(_simpleCommands[i]->_arguments[1], _simpleCommands[i]->_arguments[2], 1);
		if(error) {
			perror("setenv");
		}
		clear();
		prompt();
		return 1;
	}

	if(strcmp(_simpleCommands[i]->_arguments[0], "unsetenv") == 0){
		int error = unsetenv(_simpleCommands[i]->_arguments[1]);
		if(error) {
			perror("unsetenv");
		}
		clear();
		prompt();
		return 1;
	}

	if(strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0){
		int error;
		if(_simpleCommands[i]->_numOfArguments == 1){
			error = chdir(getenv("HOME"));
		} else {
			error = chdir(_simpleCommands[i]->_arguments[1]);
		}

		if(error < 0){
			perror("cd");
		}

		clear();
		prompt();
		return 1;
	}
	
	return 0;
}

void Command::execute() {
	// Don't do anything if there are no simple commands
	if ( _numOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

//
	if(strcmp(_simpleCommands[0]->_arguments[0],"exit") == 0){
		printf("Good bye!!\n");
		exit(1);
	}

	if (_inCounter > 1 || _outCounter > 1) {
		printf("Ambiguous output redirect.\n");
		clear();
		prompt();
		return;
	}

	// Print contents of Command data structure
//print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int tmpin = dup(0);
	int tmpout = dup(1);
	int tmperr = dup(2);

	int fdin;
	int	fdout;
	int fderr;

	if(_inFile){
		fdin = open(_inFile, O_RDONLY);
	}
	else {
		fdin = dup(tmpin);
	}	

	if(_errFile){
		if(_append){
			fderr = open(_errFile, O_WRONLY | O_APPEND | O_CREAT, 0600);
		}
		else {
			fderr = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		}
	}
	else {
		fderr = dup(tmperr);
	}

	dup2(fderr,2);
	close(fderr);

	int pid;

	for(int i = 0; i < _numOfSimpleCommands; i++){
		//built in
		if(BuiltIn(i))	return;

		dup2(fdin,0);
		close(fdin);

		if(i == _numOfSimpleCommands-1){
			if(_outFile){
				if(_append){
					fdout = open(_outFile, O_WRONLY | O_APPEND | O_CREAT, 0600);
				}
				else {
					fdout = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
				}
			}
			else {
				fdout = dup(tmpout);
			}
		} else {
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}

		dup2(fdout,1);
		close(fdout);

		pid = fork();

		if (pid == -1){
			perror("fork\n");
			exit(2);
		}

		if(pid == 0){
			if(strcmp(_simpleCommands[i]->_arguments[0], "printenv") == 0){
				char ** env = environ;

				while(*env){
					printf("%s\n", *env);
					env++;
				}
			}

			if(strcmp(_simpleCommands[i]->_arguments[0], "source") == 0){
				FILE * fp = fopen(_simpleCommands[i]->_arguments[1], "r");
                char cmdline [1024];

				fgets(cmdline, 1023, fp);
				fclose(fp);

				int tmpin = dup(0);
				int tmpout = dup(1);

				int fdpipein[2];
				int fdpipeout[2];

				pipe(fdpipein);
				pipe(fdpipeout);

				write(fdpipein[1], cmdline, strlen(cmdline));
				write(fdpipein[1], "\n", 1);

				close(fdpipein[1]);

				dup2(fdpipein[0], 0);
				close(fdpipein[0]);
				dup2(fdpipeout[1], 1);
				close(fdpipeout[1]);

				int ret = fork();
				if (ret == 0) {
					execvp("/proc/self/exe", NULL);
					_exit(1);
				} else if (ret < 0) {
					perror("fork");
					exit(1);
				}

				dup2(tmpin, 0);
				dup2(tmpout, 1);
				close(tmpin);
				close(tmpout);

				char ch;
				char * buffer = (char *) malloc (100);
				int i = 0;
	
				// Read from the pipe the output of the subshell
				while (read(fdpipeout[0], &ch, 1)) {
		 			if (ch != '\n')
		   		 	buffer[i++] = ch;
				}

				buffer[i] = '\0';
				printf("%s\n",buffer);

				} 
				else {
					execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
				//	perror("execvp");
					_exit(1);
				}
			}
		} //for

	dup2(tmpin,0);
	dup2(tmpout,1);
	dup2(tmperr,2);
	close(tmpin);
	close(tmpout);
	close(tmperr);

	if(!_background){
		waitpid(pid,NULL,0);
	}

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation


void Command::prompt() {
	char * PROMPT = getenv("PROMPT");
	char * ERR = getenv("onError");
	
	if(isatty(0) && !PROMPT){
		printf("myshell>");
		fflush(stdout);
	}

	if(ERR == NULL){
		onError = false;
	}
	if(isatty(0) && !onError && PROMPT){
		printf("%s",PROMPT);
	}
	if(isatty(0) && onError && PROMPT){
		printf("%s",ERR);
	}
	fflush(stdout);
	onError = false;
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;
