#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	// duplicate stdout descriptor to index 3 in file descriptor table (saves stdout reference to the open file table)
	int outfd = dup(STDOUT_FILENO);
	// close stdout; STDOUT_FILENO is an alias for 1
	close(STDOUT_FILENO);
	// creates a fd at index 1 in the file descriptor table; anything printed to stdout will be redirected to text.txt
	int fd = open("text.txt",O_RDWR | O_CREAT, S_IRUSR);
	// Now we create a new process to execute the command!
	pid_t pid = fork();
	// check if inside child process
	if(pid==0)
	{
		// initialize command
		char *x[2];
		x[0]="ls";
		x[1]=NULL;

		// Remember for the project you should be using execv() and not execvp()!
		// ls command is executed and output is redirected to text.txt; exits child process
		execvp(x[0],x);
	}
	// replace the reference to text.txt in the open file table at index 1 with the original reference to stdout in the parent process
	dup2(outfd, STDOUT_FILENO);
	// we can close extra reference to stdout
	close(outfd);
	// print string to stdout
	printf("Redirected output to test.txt\n");
	return 0;
}
