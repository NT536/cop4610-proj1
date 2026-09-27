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

void run_pipeline(command cmds[], int ncmds) {
    char *paths[MAX_CMDS] = {0};
    int   pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];
    int   npipes = ncmds - 1;
    int   started = 0;

    // 1. Validate and resolve everything before creating any pipes or children 
    if (cmds[0].infile && validate_input_file(cmds[0].infile) == -1)
        return;

    for (int i = 0; i < ncmds; i++) {
        paths[i] = path_search(cmds[i].argv[0]);
        if (!paths[i]) {
            fprintf(stderr, "%s: command not found\n", cmds[i].argv[0]);
            goto cleanup;
        }
    }

    // 2. Create all pipes up front so every child inherits them
    for (int i = 0; i < npipes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            close_all(pipes, i);   /* close only the ones already opened */
            goto cleanup;
        }
    }

    // 3. One child per command
    for (int i = 0; i < ncmds; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            break;
        }
        if (pids[i] == 0) {
            if (i > 0)      dup2(pipes[i - 1][0], STDIN_FILENO);   /* read from previous */
            if (i < npipes) dup2(pipes[i][1],     STDOUT_FILENO);  /* write to next */
            close_all(pipes, npipes);   /* MUST close originals or readers never see EOF */

            /* File redirects applied after pipes so they override (extra credit) */
            /* CHECK: assumes apply_redirection returns -1 on failure */
            if (apply_redirection(&cmds[i]) == -1)
                _exit(1);

            execv(paths[i], cmds[i].argv);
            perror("execv");
            _exit(1);
        }
        started++;
    }

    // 4. Parent closes every pipe end, then waits for every child
    close_all(pipes, npipes);
    for (int i = 0; i < started; i++)
        waitpid(pids[i], NULL, 0);

cleanup:
    for (int i = 0; i < ncmds; i++)
        free(paths[i]);
}