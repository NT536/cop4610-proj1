#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
int main() {
	int status;
	// what happens if you don't fork? try running execv without it!
	pid_t pid = fork();
	if (pid == 0) {
		char *argv[] = {"/bin/ls", "-l", NULL}; // might have to change /bin/ls based on path, do which ls to find where ls is located.
		// char *argv[] = {"/bin/sleep", "3", NULL}; // might have to change /bin/sleep based on path, do which sleep to find where sleep is located.
		execv(argv[0], argv);
	}
	else {
		waitpid(pid, &status, 0);
		printf("Child Complete\n");
		exit(0);
	}
}
