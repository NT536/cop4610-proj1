#include "main.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

    while(1)
    {
        printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
        char *input = get_input();
        tokenlist *tokens = get_tokens(input);

        for(int i = 0; i < tokens->count; i++)
        {
            //need to change these to add reallocation
            //environmental variable expansion
            if(tokens->items[i][0] == '$')
            {
                //turn $ into getenv
                char *copy = strdup(tokens->items[i] + 1);
                //reallocate enough space for getenv
                tokens->items[i] = realloc(tokens->items[i], strlen(getenv(copy)) + 1);
                tokens->items[i] = getenv(copy);
                free(copy);
            }
            //tilde expansion
            else if(tokens->items[i][0] == '~')
            {
                //reallocate enough space for $HOME
                tokens->items[i] = realloc(tokens->items[i], strlen(getenv("HOME")) + 1);
                //turn tilde into $HOME
                tokens->items[i] = getenv("HOME");
            }
        }

        free(input);
        free_tokens(tokens);
    }

    return 0;
}