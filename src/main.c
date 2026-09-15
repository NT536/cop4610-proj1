#include "main.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

    while(1)
    {
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
    }

    return 0;
}