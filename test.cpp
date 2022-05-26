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

// #include <istream>
// #include <ios>


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

// 		// std::string response_header =
// 		// 	int fd = open("./site/image.png", O_RDWR);
// 		// 	char buf[2048];
// 		// 	int rd;
// 		// 	int bytes = 0;
// 		// 	std::string text;
// 		// 	while ((rd = read(fd, buf, 2048)) > 0)
// 		// 	{
// 		// 		buf[rd] = 0;
// 		// 		text += buf;
// 		// 		bytes += rd;
// 		// 	}

// 	// //---------------------var 2---------------------

// 	std::fstream fs;
// 	std::string line;
// 	std::string result;
// 	fs.open ("./site/image.png", std::fstream::in);
// 	if (fs.is_open())
// 	{
// 		while (getline (fs,line))
// 				result.append(line + "\n");
// 		fs.close();
// 	}
// 	std::cout << result << "\n";
// 	std::cout << result.length() << "\n";

// 	// //---------------------var 3---------------------

// 	std::ifstream input;
// 	std::string dir_name = "./site/image.png";

// 	input.open(dir_name.c_str(), std::ios::binary|std::ios::in);
// 	if(!input.is_open())
// 		// return false;
// 		return(1);
// 	// get length of file:			https://m.cplusplus.com/reference/istream/istream/seekg/?kw=seekg
// 	input.seekg(0, std::ios::end); //seekg sets the position of the next character to be extracted from the input stream.
// 	size_t file_size = input.tellg(); //узнаем размер файла, tellg returns the position of the current character in the input stream.
// 	input.seekg(0, std::ios::beg);

// 	size_t range_begin = 0;
// 	if(range_begin && range_begin < file_size && range_begin > 0)
// 		input.seekg(range_begin); 

// 	std::string line;
// 	std::string result;
// 	while (getline (input,line))	//reading map
// 				result.append(line + "\n");
// 	input.close();
// 	std::cout << result << "\n";
// 	std::cout << result.length() << "\n";

// 	return(0);
// }