%{
#include <stdio.h>
#include <wordexp.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

enum { ENF = 0, NEWLINE, PARAM, PIPE, OUT_REDIR, IN_REDIR, };
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
[\t\r\v\n ]            ;
[$~!?"a-zA-Z0-9\-_\/\.]+ { return PARAM;}
%%

char cwd[PATH_MAX]; // Current Working Directory

void cd(char* s) {
    // using `wordexp` we also have tilde-expansion and
    // variable substitution
    wordexp_t exp_result;
    wordexp(s, &exp_result, 0);
    if (chdir(exp_result.we_wordv[0]) != 0) { perror(""); }
    wordfree(&exp_result);
}

void display_prompt() {
    // to give wish a unique feel, we let the cwd path of the prompt consist of
    // random uppercase and lowercase characters.
    getcwd(cwd, PATH_MAX);
    for (int i = 0; i < strlen(cwd); i++) {
        if (rand() % 2) {
            if (cwd[i] >= 'a' && cwd[i] < 'z') {
                cwd[i] -= 'a' - 'A';
            }
        }
    }
    printf("\n%s> ", cwd);
}

void fork_and_wait(char **argv, int argc, int stdin, int stdout) {
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            fprintf(stderr, "Could not fork.");
            exit(1);
        case 0:
            if (stdin != 0) {
                dup2(stdin, 0);
            }
            if (stdout != 1) {
                dup2(stdout, 1);
            }
            if  ((execvp(argv[0], argv) == -1)) {
                fprintf(stderr, "Error executing program %s", argv[0]);
                exit(1);
            }
            exit(0);
        default:
            if ((waitpid(pid, &status, 0) == -1)) {
                perror("Error in wait");
                exit(1);
            }
            if (WIFEXITED(status)) { child_exit_code = WEXITSTATUS(status); }
    }
}

int main(int pargc, char *pargv[])
{   
    FILE *input_stream = stdin;
    char line[1024];
    char *argv[100];
    int argc = 0;
    int token, fd_redir_in, fd_redir_out;

    if (pargc > 1){
        if((input_stream = fopen(pargv[1], "r")) == NULL){
            perror("fopen");
            exit(errno);
        }
    }

    while (1)
    {  
        display_prompt();

        YY_FLUSH_BUFFER;
        fgets(line, 1024, input_stream);

        YY_BUFFER_STATE bufstate = yy_scan_string(line);
        while ((token = yylex()))
        {
            switch (token)
            {
            case PARAM:
                if ((argv[argc] = malloc(yyleng + 1)) == NULL) {
                    fprintf(stderr, "error in malloc!");
                    exit(1);
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
        };
        argv[argc] = NULL; // terminate so we know when the array of pointers end.

        if ( argc == 0 ) {
            continue;
        }

        if (strcmp(argv[0], "cd") == 0) {
            cd(argv[1]);
        } else if (strcmp(argv[0], "exit") == 0) {
            printf("\nBye!\n");
            exit(0);
        } else {
            fork_and_wait(argv, argc, fd_redir_in, fd_redir_out);
        }

        // cleanup 
        YY_FLUSH_BUFFER;
        yy_delete_buffer(bufstate);
        fd_redir_in = 0;
        fd_redir_out = 1;
        argc = 0;
    }
}
