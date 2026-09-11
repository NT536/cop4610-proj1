#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	// create array for pipe file descriptors
	int fd[2];
	// create read and write end of pipe
	pipe(fd);
	printf("fd[0] = %d, fd[1] = %d\n", fd[0], fd[1]);
	// create new process
	pid_t pid = fork();

	// child process
	if(pid == 0) {
		// initialize command
		char *x[2];
		x[0] = "ls";
		x[1] = NULL;

		// replace stdout with write end of pipe
		dup2(fd[1], STDOUT_FILENO);
		// close read end of pipe
		close(fd[0]);
		// close duplicate reference to write end of pipe
		close(fd[1]);

		// remember to use execv() instead of execvp() for the project!
		execvp(x[0], x);
	}
	// parent process
	else {
		// initialize command
		char *x[3];
		x[0] = "wc";
		x[1] = "-l";
		x[2] = NULL;

		// replace stdin with read end of pipe
		dup2(fd[0], STDIN_FILENO);
		// close read end of pipe
		close(fd[0]);
		// close duplicate reference to write end of pipe
		close(fd[1]);

		// remember to use execv() instead of execvp() for the project!
		execvp(x[0], x);
	}
	return 0;
}
