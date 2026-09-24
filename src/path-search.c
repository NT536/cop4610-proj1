#include "path-search.h"
#include <string.h>

char*path_search(char *cmd){

    if (strchr(cmd, '/') != NULL){
        return srtdup(cmd);
    }else {
        char *path_env = getenv("PATH")
        return NULL;
    }
}