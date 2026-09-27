#ifndef PIPING_H
#define PIPING_H

#include "lexer.h"        // header that defines tokenlist
#include "redirection.h"  // command struct

#define MAX_CMDS 3        // at most 2 pipes

int  has_pipe(tokenlist *tokens);
int  split_pipeline(tokenlist *tokens, command cmds[], int *ncmds);
void run_pipeline(command cmds[], int ncmds);
void free_pipeline(command cmds[], int ncmds);

#endif