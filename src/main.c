#define _POSIX_C_SOURCE 200809L

#include "main.h"
#include "lexer.h"
#include "path-search.h"
#include "redirection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*
    For Background processes, create bool variable for if command is executed as background.
*/

int main() {

    int status;
    bool cont = true;
    while(cont == true)
    {
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getcwd(NULL, 0));
        char *input = get_input();
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
        
        // i/o redirection
        command cmd;
        if (parse_redirection(tokens->items, tokens->size, &cmd) == -1)
            continue;
        if (cmd.infile != NULL && validate_input_file(cmd.infile) == -1) {
            free_command(&cmd);
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
            waitpid(pid, &status, 0);

            //free command struct
            free_command(&cmd);
        }

        if(input != NULL)
            free(input);
        if(path != NULL)
            free(path);
        if(tokens != NULL)
            free_tokens(tokens);
    }

    return 0;
}
