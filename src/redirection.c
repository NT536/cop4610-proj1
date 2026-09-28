#define _POSIX_C_SOURCE 200809L

#include "redirection.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Permission bits for output files: -rw------- */
#define OUT_MODE (S_IRUSR | S_IWUSR)

static int is_redirect(const char *tok)
{
    return strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0;
}

int parse_redirection(char **tokens, int size, command *cmd)
{
    cmd->argc = 0;
    cmd->infile = NULL;
    cmd->outfile = NULL;
    cmd->argv = malloc((size + 1) * sizeof(char *));
    if (cmd->argv == NULL) {
        perror("malloc");
        return -1;
    }

    for (int i = 0; i < size; i++) {
        if (is_redirect(tokens[i])) {
            /* the operator must be followed by a filename */
            if (i + 1 >= size || is_redirect(tokens[i + 1])) {
                fprintf(stderr, "syntax error: missing file after '%s'\n", tokens[i]);
                free_command(cmd);
                return -1;
            }
            if (tokens[i][0] == '<')
                cmd->infile = tokens[i + 1];
            else
                cmd->outfile = tokens[i + 1];
            i++; /* skip the filename */
        } else {
            cmd->argv[cmd->argc++] = tokens[i];
        }
    }
    cmd->argv[cmd->argc] = NULL;

    if (cmd->argc == 0) {
        fprintf(stderr, "syntax error: missing command\n");
        free_command(cmd);
        return -1;
    }
    return 0;
}

int validate_input_file(const char *path)
{
    struct stat st;

    if (stat(path, &st) == -1) {
        fprintf(stderr, "%s: No such file or directory\n", path);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s: Not a regular file\n", path);
        return -1;
    }
    return 0;
}

int apply_redirection(const command *cmd)
{
    /* input first, so a bad input file never truncates the output file */
    if (cmd->infile != NULL) {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd == -1) {
            perror(cmd->infile);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->outfile != NULL) {
        int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, OUT_MODE);
        if (fd == -1) {
            perror(cmd->outfile);
            return -1;
        }
        /* O_TRUNC keeps an existing file's old mode, so force -rw------- */
        if (fchmod(fd, OUT_MODE) == -1) {
            perror("fchmod");
            close(fd);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

void free_command(command *cmd)
{
    free(cmd->argv);
    cmd->argv = NULL;
    cmd->argc = 0;
}