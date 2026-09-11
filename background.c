#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
	int status;
	pid_t pid = fork();

	if(pid == 0) {
		sleep(3);
		printf("Child process\n");
	}
	else {
		waitpid(pid, &status, WNOHANG);
		printf("Parent process\n");
	}
}
