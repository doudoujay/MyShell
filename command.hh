#ifndef command_h
#define command_h

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
	  int _numOfAvailableSimpleCommands;
	  int _numOfSimpleCommands;
	  SimpleCommand ** _simpleCommands;
      char * _outFile;
	  char * _inFile;
	  char * _errFile;
	  int _append; //>> flag
	  int _background;
	  int _inCounter;       //# of in
	  int _outCounter;      //# of out

	  void prompt();
	  void print();
	  void execute();
	  void clear();
	  int BuiltIn(int i);
	  Command();
	  void insertSimpleCommand( SimpleCommand * simpleCommand );
	  
      static Command _currentCommand;
	  static SimpleCommand *_currentSimpleCommand;
};

#endif
