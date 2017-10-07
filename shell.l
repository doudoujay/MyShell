
/*
 *
 * CS-252 Fall 2017
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

">>" {
	return GREATGREAT;
}

"2>" {
	return TWOGREAT;
}

"|" {
	return PIPE;
}

"&" {
	return AMPERSAND;
}

"<" {
	return LESS;
}

">&" {
	return GREATAMPERSAND;
}

">>&" {
	return GREATGREATAMPERSAND;
}


\`[^\n\`]*\`    {
        using namespace std;
        string entry(yytext);
        entry = entry.substr(1, entry.size() - 2);

        vector<string> argList;
        stringstream ss;
        ss.str(entry);
        string argChunk;
        while (getline(ss, argChunk, ' ')) { //Split expandable arg by its directories.
                       argList.push_back(argChunk);
        }

        //Build argument
        char* arg[argList.size() + 1];
        int i;
        for (i = 0; i < argList.size(); i++){
                arg[i] = strdup(argList[i].c_str());
        }
        arg[i] = NULL;

        int filedes[2]; //Open the pipe
        if (pipe(filedes) == -1) {
                perror("pipe");
                exit(1);
        }

		int pid = fork();
        if ( pid == 0 ) {
                while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
                close(filedes[1]);
                close(filedes[0]);
                execvp(arg[0], arg);
                perror("execvp");
                _exit(1);
        }
        waitpid(pid, NULL, 0);  // Parent process waits here for child to terminate.
        close(filedes[1]);
        char buffer[4096];
        while (1) {
                ssize_t count = read(filedes[0], buffer, sizeof(buffer));
                if (count == -1) {
                        if (errno == EINTR) {
                                continue;
                        }
                        else {
                                perror("read");
                                exit(1);
                        }
                } else if (count == 0) {
                               break;
                }
        }
        close(filedes[0]);
		entry = buffer;
        int pos = entry.find("\n");
        while(pos != string::npos){
                entry.replace(pos, 1, " ");
                pos = entry.find("\n", pos + 1);
        }
        yylval.string_val = strdup(entry.c_str());
        return WORD;

}

["][^\n\"]*["] { 
	/* quotes */
	yylval.string_val = strdup(yytext+1);

    if (yylval.string_val[yyleng-2] == '"')
		yylval.string_val[yyleng-2] = 0;
	return WORD;
} 

[^ \t\n|><&]*\\[^ \t\n]* {
	/* escaping */
//printf("yyval=%s\n",yylval.string_val);
	int i = 0;
	char * escape = (char *)malloc(200);
	char * temp = yytext;

	while(*temp){
		if (*temp == '\\'){
			if(*(temp+1) == '\\'){
				temp = temp + 2;
				escape[i++] = '\\';
			} else {
				escape[i++] = *(++temp);
			} 
		} else {
			escape[i++] = *temp;
		}
		temp++;
	}

	escape[i] = '\0';
	yylval.string_val = strdup(escape);
	return WORD;
}

[^ \t\n][^ \t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.string_val = strdup(yytext);
  return WORD;
}
