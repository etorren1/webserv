#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <ios>


#define BUF_SIZE 4048

// int main (int argc, char **argv, char **envp)
int main ()
{
	// std::stringstream reader;
	// char buf[BUF_SIZE + 1];
    // int rd = 0;
	// long reader_size = 0;

	// int fd = open("new.txt", O_RDWR | O_CREAT | O_APPEND, 0777);

    // while ((rd = read(0, buf, BUF_SIZE)) > 0) {
    //     buf[rd] = 0;
	// 	std::cerr << "\e[34;1mCGIRead(" << rd << "): \e[0m\n";// << buf;
	// 	write(fd, buf, rd);
    //     reader << buf;
    //     reader_size += rd;
    // }
	
	// // std::cout << "\e[31m" << reader.str();
	// return 20;

	// std::fstream _file;
	std::fstream _file;
		// _file.open("some.txt", std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc); // open file
		_file.open("site/my_post_file.txt", std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
	if (_file.is_open())
		std::cout << "Success\n";
	else
		std::cout << "Failture\n";
	return (0);
}