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

using namespace std;

int numArgs = 10000;
int curArg = 1;

int _histCount = 0; //Used to count lines
string arg = "";
bool onError = false;

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


void
SimpleCommand::expandWildcardsIfNecessary(vector<string> pre, char * arg){
	string argument(arg);
	
	if(pre.size() == 0 && argument.find("*") == string::npos && argument.find("?") == string::npos ){
		numArgs = 1;
		arg = strdup(argument.c_str());
		Command::_currentSimpleCommand->insertArgument(arg);
	}
	else{ //Contains * or ? wildcard.
		//My implementation
		vector<string> dirList;
		string startDir;
		int curDir = 0;
		
		if(pre.size() == 0){
			stringstream ss;
			ss.str(argument);
			string dirChunk;
			getline(ss,dirChunk, '/');
			while (getline(ss, dirChunk, '/')) { //Split expandable arg by its directories.
			       dirList.push_back(dirChunk);
			}

			int count = 0;
			for(curDir = 0; curDir < dirList.size(); curDir ++) {
				if(dirList[curDir].find("*") == string::npos && dirList[curDir].find("?") == string::npos ){
					//If dirChunk doesnt need expanded, add it to start dir	
					if(count++ == 0){
						startDir.push_back('/');
					}
					startDir.append(dirList[curDir]);
					startDir.push_back('/');
				}
				else{
					if(curDir == 0){
						startDir = "/";
					}
					argument = "";
					argument.append(dirList[curDir]);
					break; //Only handles one dir
				}
			}  //End For 
		} //End if pre doesnt exist
		
		else{ //Pre exists
				if(pre.size() > 1){	
					startDir.append(pre[2]);
				} else{
					startDir.append(pre[0]);
				}
				startDir.append("/");
		}
		//printf("startdir-> %s arg-> %s\n", startDir.c_str(), argument.c_str());	
		//Slide 266
		arg = strdup(argument.c_str());
		char * reg = (char*)malloc(2*strlen(arg)+10);
		char * a = arg;
		char * r = reg;
		*r = '^'; r++;
		while(*a){
			if(*a == '*') { *r = '.'; r++; *r = '*'; r++; }
			else if (*a == '?') { *r = '.'; r++; }
			else if (*a == '.') { *r = '\\'; r++; *r = '.'; r++; }
			else { *r = *a; r++; }
			a++;
		}
		*r = '$'; r++; *r = 0; //match end of line and add the null
		
		//Compile Regex
		regex_t regex;	
		if( regcomp( &regex, reg,  REG_EXTENDED|REG_NOSUB) != 0 ){
			onError = true;
			perror("compile");
		}

		//List Directories
		//TODO update "." to handle any directory
		DIR * dir;
		//dir = opendir(startDir);
		if(startDir.compare("") == 0){
			dir = opendir(".");
		}
		else{
			dir = opendir(startDir.c_str());
		}
		vector<string> results;
		if (dir == NULL){
			onError = true;
			perror("opendir");
			return;
		}



		
		else{
			struct dirent * ent;
			vector<string> longResults;
			while(( ent = readdir(dir)) != NULL){
				if(regexec(&regex, ent->d_name, 0, NULL, 0) == 0){
					string tmpDir = startDir;
					tmpDir.append(ent->d_name);
					longResults.push_back(tmpDir);
					results.push_back(ent->d_name); //Add every result to the vector
				}		
			}
			sort(results.begin(),results.end()); //Sort the vector
			numArgs = results.size();

			if(dirList.size() != 0 && curDir < dirList.size() - 1 ){
				char * newArg = strdup(dirList[++curDir].c_str());
				expandWildcardsIfNecessary(longResults, newArg);
				return;
			}
			for(int i = 0 ; i< results.size(); i++){
				string sDir = startDir; //Adds inital dir
				sDir.append(results[i]); //Appends this result

				char * toAdd = strdup(sDir.c_str());		
				if(results[i][0] != '.'){
					Command::_currentSimpleCommand->insertArgument(toAdd); //Add every result as an argument
				}
			}
		}
		closedir(dir);

	}//End wildcard expandsion
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

			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
			perror("execvp");
			_exit(1);
		}
	}

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
	if(isatty(0)){
		printf("myshell>");
		fflush(stdout);
	}
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;
