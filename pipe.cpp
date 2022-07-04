#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>

#define BUF_SIZE 4048

int main (int argc, char **argv, char **envp)
{
	std::stringstream reader;
	char buf[BUF_SIZE + 1];
    int rd = 0;
	long reader_size = 0;

	int fd = open("new.txt", O_RDWR | O_CREAT | O_APPEND, 0777);

    while ((rd = read(0, buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
		std::cerr << "\e[34;1mCGIRead(" << rd << "): \e[0m\n";// << buf;
		write(fd, buf, rd);
        reader << buf;
        reader_size += rd;
    }
	
	// std::cout << "\e[31m" << reader.str();
	return 20;
}