// #include <iostream>
// #include <netdb.h>
// #include <poll.h>
// #include <unistd.h>
// #include <vector>
// #include <list>
// #include <fcntl.h>
// #include <string>
// #include <sstream>
// #include <ctime>
// #include <fstream>

// int main ()
// {
// 	// int fd = open("./site/image.png", O_RDWR);
// 	// char buf[2048];
// 	// int rd;
// 	// int bytes = 0;
// 	// std::string text;
// 	// while ((rd = read(fd, buf, 2047)) > 0)
// 	// {
// 	// 	buf[rd] = '\0';
// 	// 	text += buf;
// 	// 	bytes += rd;
// 	// }
// 	// std::cout << bytes << "\n";
// 	// std::cout << text.length() << "\n";

// 	std::fstream fs;
// 	std::string line;
// 	std::string result;
// 	fs.open ("./site/image.png", std::fstream::in);
// 	if (fs.is_open())
// 	{
// 		while (getline (fs,line))	//reading map
// 				result.append(line + "\n");
// 		fs.close();
// 	}
// 	std::cout << result << "\n";
// 	std::cout << result.length() << "\n";


// }