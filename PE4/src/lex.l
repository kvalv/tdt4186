%{
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

enum { ENF = 0, PARAM, PIPE, OUT_REDIR, IN_REDIR, };
int child_exit_code;
%}

%option noyywrap
%option nounput
%option noinput

%%
$? { printf("%d\n", child_exit_code); return 0; }
\< { return IN_REDIR; }
\> { return OUT_REDIR; }
\| { return PIPE; }
[\t\n\r\v ]            ;
[!?"a-zA-Z0-9\-_\/]+ { return PARAM;}
%%

int main (int pargc, char **pargv) {
    char line[1024];
    char *argv[100];
    int argc = 0;

    int token, fd_redir_in, fd_redir_out;
    while (1) {
        printf("\n:: ");
        fgets(line, 1024, stdin);
        YY_BUFFER_STATE bufstate = yy_scan_string(line);
        while ((token = yylex())) {
            switch(token) {
                case PARAM:
                    if ((argv[argc] = malloc(yyleng + 1)) == NULL) {
                        fprintf(stderr, "error in malloc!"); exit(1);
                    }
                    strcpy(argv[argc++], yytext);
                    break;
                case IN_REDIR:
                    yylex();
                    if ((fd_redir_in = open(yytext, O_RDONLY)) < 0) {
                        exit(errno);
                    }
                    break;
                case OUT_REDIR:
                    yylex();
                    if ((fd_redir_out = open(yytext, O_WRONLY | O_CREAT, 0666)) < 0) {
                        exit(errno);
                    }
                    break;

            }
        }
        argv[argc] = NULL;  // terminate so we know when the array of pointers end.

        pid_t pid = fork();
        int status;
        switch(pid) {
            case -1:
                fprintf(stderr, "Could not fork.");
                exit(1);
            case 0:
                if ( fd_redir_in != 0 ) {
                    dup2( fd_redir_in, 0);
                }
                if ( fd_redir_out != 1 ) {
                    dup2( fd_redir_out, 1 );
                }
                if ((execvp(argv[0], argv) == -1)) {
                    perror("Error executing program");
                    exit(1);
                }
                exit(0);
            default:
                if ( ( waitpid(pid, &status, 0) == -1) ) {
                    perror("Error in wait"); exit(1);
                }
                if (WIFEXITED(status)) { 
                    child_exit_code = WEXITSTATUS(status);
                }
        }

        /* cleanup */
        YY_FLUSH_BUFFER;
        yy_delete_buffer(bufstate);
        fd_redir_in = 0;
        fd_redir_out = 1;
        argc = 0;
    }
}
