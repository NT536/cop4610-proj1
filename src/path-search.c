#define _POSIX_C_SOURCE 200809L

#include "path-search.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

char *path_search(char *cmd)
{
    if (strchr(cmd, '/') != NULL)
    {
        return strdup(cmd);
    }

    char *path_env = getenv("PATH");
    if (path_env == NULL)
    {
        return NULL;
    }

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");
    char full_path[1024];

    while (dir != NULL)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}
