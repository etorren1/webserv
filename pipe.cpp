#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

#define PATH_INFO "cgi_tester"
#define PIPE_IN 1	//we write
#define PIPE_OUT 0	//we read

int main (int argc, char **argv, char **envp)
{
	// // ------------------------ 1 ------------------------
	char buff[100];
	for (int i = 0; i < 100; i++)
		buff[i] = 0;
	std::string str= "HeLlO WoRlD! bla bla bla dHgfvbUJYIKJghgvjdfiUBUkjhGnhJ234234gdfgdfg";
	int pid;
	int pipe1[2];
	int pipe2[2];
	int ex;
	int status;

	std::stringstream reqBody;
	reqBody << str;

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
		if ((ex = execve(PATH_INFO, NULL, envp)) < 0)
			return 1;
		exit(ex);
	}
	else //parent - current programm prosses
	{
		close(pipe1[PIPE_OUT]);		//Close unused pipe read end
		close(pipe2[PIPE_IN]); //Close unused pipe write end
		write(pipe1[PIPE_IN], str.c_str(), str.length());
		close(pipe1[PIPE_IN]);	
		waitpid(pid, &status, 0);
		read(pipe2[PIPE_OUT], &buff, 100);
		write(1, &buff, 100);
	}


	// //------------------------ 2 ------------------------
	// char buff[100];
	// std::stringstream stream;
	// // for (int i = 0; i < 100; i++)
	// // 	buff[i] = 0;
	// stream << "bla bla bla";
	// stream.read(buff, 100);
	// std::cout << buff[13];

	return 0;
}