#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "piping.h"
#include "path-search.h"


int has_pipe(tokenlist *tokens) {
    for (int i = 0; i < tokens->size; i++)
        if (strcmp(tokens->items[i], "|") == 0)
            return 1;
    return 0;
}

void free_pipeline(command cmds[], int ncmds) {
    for (int i = 0; i < ncmds; i++)
        free_command(&cmds[i]);
}

// Split tokens on "|" into up to MAX_CMDS commands.
int split_pipeline(tokenlist *tokens, command cmds[], int *ncmds) {
    int start = 0, n = 0;

    for (int i = 0; i <= tokens->size; i++) {
        if (i < tokens->size && strcmp(tokens->items[i], "|") != 0)
            continue;

        int len = i - start;
        if (len == 0 || n == MAX_CMDS) {
            fprintf(stderr, "syntax error near '|'\n");
            free_pipeline(cmds, n);
            return -1;
        }
      
        if (parse_redirection(tokens->items + start, len, &cmds[n]) == -1) {
            free_pipeline(cmds, n);
            return -1;
        }
        n++;
        start = i + 1;
    }

   for (int i = 0; i < n; i++) {
        if ((cmds[i].infile && i != 0) || (cmds[i].outfile && i != n - 1)) {
            fprintf(stderr, "syntax error: redirection inside pipeline\n");
            free_pipeline(cmds, n);
            return -1;
        }
    }

    *ncmds = n;
    return 0;
}

static void close_all(int pipes[][2], int npipes) {
    for (int j = 0; j < npipes; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
}

// Runs a pipeline of ncmds commands.
// Foreground: waits for every child.
// Background: returns without waiting and copies child PIDs into pids_out.
// Returns ncmds on success, -1 if nothing was run.

int run_pipeline(command *cmds, int ncmds, int background, pid_t *pids_out) {
    char *paths[MAX_CMDS] = {0};
    int pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];

    // input file only allowed on first command 
    if (cmds[0].infile != NULL && validate_input_file(cmds[0].infile) == -1)
        return -1;

    // resolve every command before creating anything
    for (int i = 0; i < ncmds; i++) {
        paths[i] = path_search(cmds[i].argv[0]);
        if (paths[i] == NULL) {
            fprintf(stderr, "%s: command not found\n", cmds[i].argv[0]);
            for (int k = 0; k < i; k++)
                free(paths[k]);
            return -1;
        }
    }

    // create all pipes
    for (int i = 0; i < ncmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            for (int k = 0; k < i; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }
            for (int k = 0; k < ncmds; k++)
                free(paths[k]);
            return -1;
        }
    }

    // fork one child per command
    int started = 0;
    for (int i = 0; i < ncmds; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i > 0)
                dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < ncmds - 1)
                dup2(pipes[i][1], STDOUT_FILENO);
            for (int k = 0; k < ncmds - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }
            if (apply_redirection(&cmds[i]) == -1)   // after pipe dup2s
                exit(1);
            execv(paths[i], cmds[i].argv);
            perror("execv");
            exit(1);
        } else if (pids[i] < 0) {
            perror("fork");
            break;
        }
        started++;
    }

    // parent closes every pipe end
    for (int k = 0; k < ncmds - 1; k++) {
        close(pipes[k][0]);
        close(pipes[k][1]);
    }

    if (started < ncmds) {
        // a fork failed: clean up what did start, don't make a job
        for (int i = 0; i < started; i++)
            waitpid(pids[i], NULL, 0);
        for (int k = 0; k < ncmds; k++)
            free(paths[k]);
        return -1;
    }

    if (background) {
        for (int i = 0; i < ncmds; i++)
            pids_out[i] = pids[i];
    } else {
        for (int i = 0; i < ncmds; i++)
            waitpid(pids[i], NULL, 0);        // specific PIDs only
    }

    for (int k = 0; k < ncmds; k++)
        free(paths[k]);
    return ncmds;
}

