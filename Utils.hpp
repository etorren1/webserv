#ifndef Utils_hpp
#define Utils_hpp

#include <iostream>
#include <vector>

#define RESET "\e[0m"
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define PURPLE "\e[35m"
#define CYAN "\e[36m"
#define GRAY "\e[37m"

#if __APPLE__
	#define IRC_NOSIGNAL SO_NOSIGPIPE
#else
	#define IRC_NOSIGNAL MSG_NOSIGNAL
#endif


std::vector<std::string>	split(std::string str, std::string delimiter);
std::string					trim(std::string str, std::string cut);
std::string					itos( int const & num );
bool 						existDir(const char * name);

struct fileException : public std::exception
{
	const char * what () const throw ()
	{
		return "Error while opnening file";
	}
};

#endif
