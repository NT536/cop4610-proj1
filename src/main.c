#include "main.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include "redirection.h"
#include "piping.h"

int main() {

    int status;
    while(1)
    {
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
        char *input = get_input();
        tokenlist *tokens = get_tokens(input);

        for(int i = 0; i < tokens->size; i++)
        {
            //need to change these to add reallocation
            //environmental variable expansion
            if(tokens->items[i][0] == '$')
            {
                //turn $ into getenv
                char *copy = strdup(tokens->items[i] + 1);
                //reallocate enough space for getenv
                tokens->items[i] = realloc(tokens->items[i], strlen(getenv(copy)) + 1);
                tokens->items[i] = getenv(copy);
                free(copy);
            }
            //tilde expansion
            else if(tokens->items[i][0] == '~')
            {
                //reallocate enough space for $HOME
                tokens->items[i] = realloc(tokens->items[i], strlen(getenv("HOME")) + 1);
                //turn tilde into $HOME
                tokens->items[i] = getenv("HOME");
            }
        } 
        // piping
        if (has_pipe(tokens)) {
            command cmds[MAX_CMDS];
            int ncmds;
            if (split_pipeline(tokens, cmds, &ncmds) == 0) {
                run_pipeline(cmds, ncmds);
                free_pipeline(cmds, ncmds);
            }
            continue;
}
        // i/o redirection
        command cmd;
        if (parse_redirection(tokens->items, tokens->size, &cmd) == -1)
            continue;
        if (cmd.infile != NULL && validate_input_file(cmd.infile) == -1) {
            free_command(&cmd);
            continue;
        }

        pid_t pid = fork();
        if(pid == 0)
        {
            //child process
            if (apply_redirection(&cmd) == -1)   //added
                exit(1);
            execvp(tokens->items[0], tokens->items);
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
            waitpid(pid, &status, 0);

            //free command struct
            free_command(&cmd);
        }

        free(input);
        free_tokens(tokens);
    }

    return 0;
}