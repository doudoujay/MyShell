#include <cstdlib>

#include "simpleCommand.hh"

using namespace std; 

int numArgs = 10000;
int curArg = 1;

SimpleCommand::SimpleCommand() {
	// Create available space for 5 arguments
	_numOfAvailableArguments = 5;
	_numOfArguments = 0;
	_arguments = (char **) malloc( _numOfAvailableArguments * sizeof( char * ) );
}

void SimpleCommand::insertArgument( char * argument ) {
	if ( _numOfAvailableArguments == _numOfArguments  + 1 ) {
		// Double the available space
		_numOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numOfArguments + 1] = NULL;
	
	_numOfArguments++;
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
