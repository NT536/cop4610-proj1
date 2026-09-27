#ifndef REDIRECTION_H
#define REDIRECTION_H

// One parsed command: arguments plus optional I/O redirection targets.
typedef struct {
    char **argv;    
    int argc;      
    char *infile;   
    char *outfile;  
} command;

// Splits tokens into argv/infile/outfile.
int parse_redirection(char **tokens, int size, command *cmd);

// Checks that an input file exists and is a regular file. Returns 0 if valid, -1 if not.
int validate_input_file(const char *path);

//Called in the child after fork():
int apply_redirection(const command *cmd);

// Frees the argv array
void free_command(command *cmd);

#endif