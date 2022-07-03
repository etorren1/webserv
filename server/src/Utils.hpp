#ifndef Utils_hpp
#define Utils_hpp

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

#define RESET "\e[0m"
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define PURPLE "\e[35m"
#define CYAN "\e[36m"
#define GRAY "\e[37m"

//for CGI:
#define PIPE_IN 0	//we write
#define PIPE_OUT 1	//we read
// #define CGI_PATH "/Users/etorren/sand/webserv/pipe_bin"
#define CGI_PATH "/Users/etorren/sand/webserv/cgi_tester"
#define BUF 4048   //можно заменить на общий buff

class Client;

std::vector<std::string>	split(std::string str, std::string delimiter, std::string trimer = "");
std::string					trim(std::string str, std::string cut);
bool 						existDir(const char * name);
std::string					itos( long long const & num );
void						rek_mkdir( std::string path);
std::string 				getstr(char *c, size_t size);
size_t 						find_CRLN( char* buf, size_t size, size_t indent = 0 );
size_t 						find_2xCRLN( char* buf, size_t size, size_t indent = 0 );
std::string					getCurTime(); //https://stackoverflow.com/questions/7548759/generate-a-date-string-in-http-response-date-format-in-c
long long					getFileSize(const char *fileLoc);
long						hexadecimalToDecimal(std::string hex_val);
long						getStrStreamSize(std::stringstream &strm);
void						clearStrStream(std::stringstream &strstring);
void						rek_mkdir( std::string path);
time_t						timeChecker( );

struct codeException : public std::exception
{
	std::string error;
	codeException(int code) : errorCode(code) {
		error = "Error while opening file with code " + itos(errorCode);
	}
	virtual ~codeException() throw () {}
	int errorCode;
	const char * what () const throw ()
	{
		return error.c_str();
	}
	int getErrorCode() { return(errorCode); }
};

#endif