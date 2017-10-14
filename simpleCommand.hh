#ifndef simplcommand_h
#define simplecommand_h

//C++
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "y.tab.hh"

//C
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#define MAXFILENAME 1024

struct SimpleCommand {
  // Available space for arguments currently preallocated
  int _numOfAvailableArguments;

  // Number of arguments
  int _numOfArguments;
  char ** _arguments;

  SimpleCommand();
  void insertArgument( char * argument );
  char * checkExpansion(char * argument);
  char * tilde(char * argument);
};

#endif
