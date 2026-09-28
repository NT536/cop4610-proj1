#define _POSIX_C_SOURCE 200809L

#include "main.h"
#include "lexer.h"
#include "path-search.h"
#include "redirection.h"
#include "piping.h"
#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


// 1 = background (trailing "&" removed), 0 = foreground, -1 = misplaced '&' */
static int strip_background(tokenlist *tokens) {
    int n = (int)tokens->size;
    for (int i = 0; i < n - 1; i++)
        if (strcmp(tokens->items[i], "&") == 0)
            return -1;
    if (n == 0 || strcmp(tokens->items[n - 1], "&") != 0)
        return 0;
    free(tokens->items[n - 1]);
    tokens->items[n - 1] = NULL;
    tokens->size--;
    return tokens->size == 0 ? -1 : 1;
}

// Strips the trailing '&' and surrounding whitespace from the saved command line. */
static void trim_cmdline(char *line) {
    char *amp = strrchr(line, '&');
    if (amp)
        *amp = '\0';
    size_t n = strlen(line);
    while (n > 0 && (line[n - 1] == ' ' || line[n - 1] == '\t' || line[n - 1] == '\n'))
        line[--n] = '\0';
    char *s = line;
    while (*s == ' ' || *s == '\t')
        s++;
    memmove(line, s, strlen(s) + 1);
}

/*
    For Background processes, create bool variable for if command is executed as background.
*/

int main() {

    int status;
    bool cont = true;
    while(cont == true)
    {
        check_jobs();
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getcwd(NULL, 0));
        char *input = get_input();
        char *cmdline = strdup(input);
        tokenlist *tokens = get_tokens(input);

        for(int i = 0; i < tokens->size; i++)
        {
            //need to change these to add reallocation
            //environmental variable expansion
            if(tokens->items[i][0] == '$')
            {
                //turn $ into getenv
                char *copy = malloc(strlen(tokens->items[i]));
                strcpy(copy, tokens->items[i] + 1);
                //reallocate enough space for getenv
                tokens->items[i] = realloc(tokens->items[i], strlen(getenv(copy)) + 1);
                /*there is a bug here where when later freeing the tokens, 
                assigning tokens->items[i] to getenv(copy) causes the freeing memory
                to abort as it's an 'invalid pointer'.
                */
                tokens->items[i] = getenv(copy);
                tokens->items[strlen(getenv(copy))] = NULL;
                free(copy);
            }
            //tilde expansion
            else if(tokens->items[i][0] == '~')
            {
                //reallocate enough space for $HOME
                tokens->items[i] = (char *)realloc(tokens->items[i], strlen(getenv("HOME")) + 1);
                //turn tilde into $HOME
                tokens->items[i] = getenv("HOME");
            }
        } 
        // detect &
        int bg = strip_background(tokens);
        if (bg == -1) {
            fprintf(stderr, "syntax error near '&'\n");
            free(cmdline);
            continue;
        }
        if (bg == 1) {
            if (jobs_full()) {
                fprintf(stderr, "too many background jobs\n");
                free(cmdline);
                continue;
            }
            trim_cmdline(cmdline);
        
        }
        // piping
        if (has_pipe(tokens)) {
            command cmds[MAX_CMDS];
            int ncmds;
            if (split_pipeline(tokens, cmds, &ncmds) == 0) {
                pid_t pids[MAX_CMDS];
                int n = run_pipeline(cmds, ncmds, bg, pids);
                if (bg && n > 0) {
                    add_job(pids, n, cmdline);
                    cmdline = NULL;              /* job table owns it now */
                }
                free_pipeline(cmds, ncmds);
            }
            free(cmdline);
            continue;
        }

        // i/o redirection
        command cmd;
        if (parse_redirection(tokens->items, tokens->size, &cmd) == -1) {
            free(cmdline);
            continue;
        }
        if (cmd.infile != NULL && validate_input_file(cmd.infile) == -1) {
            free_command(&cmd);
            free(cmdline);
            continue;
        }
        char *path = path_search(tokens->items[0]);
        pid_t pid = fork();
        char *changedir = "cd";
        char *exitcmd = "exit";
        if(pid == 0)
        {
            //child process
            if (apply_redirection(&cmd) == -1)   //added
                exit(1);
            execvp(cmd.argv[0], cmd.argv);
            perror("execvp failed");
            exit(1);
        }
        else if(pid < 0)
        {
            perror("fork failed");
        }
        else
        {
            //parent process
            if(strcmp(tokens->items[0], changedir) == 0)
            {
                if(tokens->items[1] == NULL || tokens->size > 2)
                {
                    perror("cd: wrong number of arguments");
                }
                else
                {
                    if(chdir(tokens->items[1]) != 0)
                    {
                        perror("chdir failed");
                    }
                }
            }
            else if(strcmp(tokens->items[0], exitcmd) == 0)
            {
                cont = false;
            }
           if (bg) {
                add_job(&pid, 1, cmdline);
                cmdline = NULL;                      // job table owns it now 
            } else {
                waitpid(pid, &status, 0);
            }

            //free command struct
            free_command(&cmd);
        }

        if(input != NULL)
            free(input);
        if(path != NULL)
            free(path);
        if(tokens != NULL)
            free_tokens(tokens);
        free(cmdline);
    }

    return 0;
}
