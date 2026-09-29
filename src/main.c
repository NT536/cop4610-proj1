#define _POSIX_C_SOURCE 200809L

#include "main.h"
#include "lexer.h"
#include "path-search.h"
#include "redirection.h"
#include "piping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    int status, jobnum = 1;
    bool cont = true, background = false;
    char **pastcommands = malloc(3 * sizeof(char *));
    for(int i = 0; i < 3; i++) {
        pastcommands[i] = malloc(1);
    }
    background_process *background_pids[10];
    for(int i = 0; i < 10; i++) {
        background_pids[i] = malloc(sizeof(background_process));
        background_pids[i]->active = false;
    }
    while(cont == true)
    {
        background = false;
        //check for finished background processes
        for(int i = 0; i < 10; i++) {
            if(background_pids[i] != NULL && background_pids[i]->active) {
                int wstatus;
                if(waitpid(background_pids[i]->pid, &wstatus, WNOHANG)) {
                    background_pids[i]->status = WEXITSTATUS(wstatus);
                    background_pids[i]->active = false;
                    jobnum--;
                }
                if(WIFEXITED(wstatus)) {
                    printf("Background process %d finished with status %d\n", background_pids[i]->job, background_pids[i]->status);
                }
            }
        }
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getcwd(NULL, 0));
        char *input = get_input();
        //store the command
        for(int i = 2; i > 0; i--) {
            pastcommands[i] = realloc(pastcommands[i], strlen(pastcommands[i - 1]) + 1);
            strcpy(pastcommands[i], pastcommands[i - 1]);
        }
        pastcommands[0] = realloc(pastcommands[0], strlen(input) + 1);
        strcpy(pastcommands[0], input);
        //get tokens
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
                char* ocopy = getenv(copy);
                strcpy(tokens->items[i], ocopy);
                tokens->items[strlen(getenv(copy))] = NULL;
                free(copy);
            }
            //tilde expansion
            else if(tokens->items[i][0] == '~')
            {
                //reallocate enough space for $HOME
                tokens->items[i] = (char *)realloc(tokens->items[i], strlen(getenv("HOME")) + 1);
                //turn tilde into $HOME
                char *copy = getenv("HOME");
                strcpy(tokens->items[i], copy);
            }
        } 
        //check for background processes
        if(tokens->size > 0 && strcmp(tokens->items[tokens->size - 1], "&") == 0) {
            background = true;
            tokens->size--; // remove the '&' from the token list
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
        char *path = path_search(tokens->items[0]);
        pid_t pid = fork();
        char *changedir = "cd";
        char *exitcmd = "exit";
        char *jobcmd = "jobs";
        if(pid == 0)
        {
            //child process
            if(path == NULL)
            {
                perror("command not found");
                exit(1);
            }
            else if (apply_redirection(&cmd) == -1)   //added
                exit(1);
            execv(path, cmd.argv);
            perror("execv failed");
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
            else if(strcmp(tokens->items[0], jobcmd) == 0 && tokens->size == 1) {
                for(int i = 0; i < 10; i++) {
                    if(background_pids[i] != NULL && background_pids[i]->active) {
                        printf("Job %d: PID %d\n", background_pids[i]->job, background_pids[i]->pid);
                    }
                }
            }
            //if background process, allow for parent to continue without waiting
            if(background) {
                waitpid(pid, &status, WNOHANG);
            } else {
                waitpid(pid, &status, 0);
            }
            //free command struct
            free_command(&cmd);
        }
        //add pid to background process list if it's a background process
        if(background) {
            for(int i = 0; i < 10; i++) {
                if(background_pids[i] != NULL && !background_pids[i]->active) {
                    background_pids[i]->pid = pid;
                    background_pids[i]->active = true;
                    background_pids[i]->status = status;
                    background_pids[i]->job = jobnum++;
                    break;
                }
            }
        }

        if(input != NULL)
            free(input);
        if(path != NULL)
            free(path);
        if(tokens != NULL)
            free_tokens(tokens);
    }
    //list out past three commands
    for(int i = 0; i < 3; i++) {
        if(pastcommands[i] != NULL && strlen(pastcommands[i]) > 0) {
            printf("Past command %d: %s\n", i + 1, pastcommands[i]);
        }
    }
    //while background processes are active, wait for them to finish
    bool finished = false;
    while(!finished) {
        finished = true;
        for(int i = 0; i < 10; i++) {
            if(background_pids[i] != NULL && background_pids[i]->active) {
                int wstatus;
                if(waitpid(background_pids[i]->pid, &wstatus, WNOHANG))
                {
                    background_pids[i]->active = false;
                    background_pids[i]->status = wstatus;
                }
                if(WIFEXITED(wstatus)) {
                    printf("Background process %d finished with status %d\n", background_pids[i]->job, background_pids[i]->status);
                }
            }
            sleep(1);
        }
        for(int i = 0; i < 10; i++) {
            if(background_pids[i] != NULL && background_pids[i]->active) {
                finished = false;
            }
        }
    }

    //free background process structs
    for(int i = 0; i < 10; i++) {
        if(background_pids[i] != NULL) {
            free(background_pids[i]);
        }
    }

    return 0;
}

    
