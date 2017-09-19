#
# CS252 - Shell Project
#
#Use GNU compiler
cc= gcc
CC= g++
ccFLAGS= -g -std=c11
CCFLAGS= -g -std=c++17
WARNFLAGS= -Wall -Wextra -pedantic

LEX=lex -l
YACC=yacc -y -d -t --debug

EDIT_MODE_ON=

ifdef EDIT_MODE_ON
	EDIT_MODE_OBJECTS=tty-raw-mode.o read-line.o
endif

all: git-commit shell cat_grep ctrl-c regular

lex.yy.o: shell.l 
	$(LEX) -o lex.yy.cc shell.l
	$(CC) $(CCFLAGS) -c lex.yy.cc

y.tab.o: shell.y
	$(YACC) -o y.tab.cc shell.y
	$(CC) $(CCFLAGS) -c y.tab.cc

command.o: command.cc command.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c command.cc

simpleCommand.o: simpleCommand.cc simpleCommand.hh
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c simpleCommand.cc

shell.o: shell.cc
	$(CC) $(CCFLAGS) $(WARNFLAGS) -c shell.cc

shell: y.tab.o lex.yy.o shell.o command.o simpleCommand.o $(EDIT_MODE_OBJECTS)
		$(CC) $(CCFLAGS) $(WARNFLAGS) -o shell lex.yy.o y.tab.o shell.o command.o simpleCommand.o $(EDIT_MODE_OBJECTS) -lfl

cat_grep: cat_grep.cc
	$(CC) $(CCFLAGS) $(WARNFLAGS) -o cat_grep cat_grep.cc

ctrl-c: ctrl-c.cc
	$(CC) $(CCFLAGS) $(WARNFLAGS) -o ctrl-c ctrl-c.cc

regular: regular.cc
	$(CC) $(CCFLAGS) $(WARNFLAGS) -o regular regular.cc 

tty-raw-mode.o: tty-raw-mode.c
	$(cc) $(ccFLAGS) $(WARNFLAGS) -c tty-raw-mode.c

read-line.o: read-line.c
	$(cc) $(ccFLAGS) $(WARNFLAGS) -c read-line.c

keyboard-example: keyboard-example.c tty-raw-mode.o
	$(cc) $(ccFLAGS) $(WARNFLAGS) -o keyboard-example keyboard-example.c tty-raw-mode.o

read-line-example: read-line-example.c tty-raw-mode.o read-line.o
	$(cc) $(ccFLAGS) $(WARNFLAGS) -o read-line-example read-line-example.c tty-raw-mode.o read-line.o

.PHONY: git-commit
git-commit:
	git checkout master >> .local.git.out || echo
	git add *.cc *.h *.l *.y Makefile >> .local.git.out  || echo
	git add test-shell/testall.out >> .local.git.out  || echo
	git commit -a -m 'Commit' >> .local.git.out || echo
	git push origin master

.PHONY: clean
clean:
	rm -f lex.yy.cc y.tab.cc y.tab.hh shell ctrl-c regular cat_grep keyboard-example read-line-example *.o

