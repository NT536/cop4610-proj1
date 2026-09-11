#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	// show example of string tokenization
	char x[] = "ls -l -a";
	char *y = strtok(x, " ");
	while(y != NULL) {
		printf("%s\n", y);
		y = strtok(NULL, " ");
	}
	return 0;
}
