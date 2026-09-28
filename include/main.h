#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <sys/types.h>

//
typedef struct{
    pid_t pid;
    bool active;
    int status;
    int job;
} background_process;  

#endif