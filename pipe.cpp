#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define PATH_INFO "cgi_tester"
#define PIPE_IN 1	//we write
#define PIPE_OUT 0	//we read

int main ()
{
	char *str = "sjhdgfjsdhfgjsdhfgjs";
	int pid;
	int pipe1[2];
	int pipe2[2];
	int ex;
	int status;

	if (pipe(pipe1) && pipe(pipe2))
		return 1;
	if ((pid = fork()) < 0)
		return 1;
	if (pid == 0) //child - prosses for CGI programm
	{
		char *buf[2048]; 
		close(pipe1[PIPE_IN]); //Close unused pipe write end
		close(pipe2[PIPE_OUT]); 
		dup2(pipe1[PIPE_OUT], 0);
		dup2(pipe2[PIPE_IN], 1);
		if ((ex = execve(PATH_INFO, NULL, NULL)) < 0)
			return 1;
		exit(ex);
	}
	else //parent - current programm prosses
	{
		close(pipe1[PIPE_OUT]);		//Close unused pipe read end
		waitpid(pid, &status, 0);
	}

	return 0;
}