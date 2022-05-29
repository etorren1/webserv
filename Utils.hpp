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
std::string					itos( long long const & num );
// std::string					getTime(); //https://stackoverflow.com/questions/7548759/generate-a-date-string-in-http-response-date-format-in-c
long long					getFileSize(const char *fileLoc);

struct codeException : public std::exception
{
	codeException(int code) : errorCode(code) {}
	int errorCode;
	const char * what () const throw ()
	{
		return "Error while opnening file";
	}
	int getErrorCode() { return(errorCode); }
};

#endif
