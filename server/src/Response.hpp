#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Utils.hpp"
#include "Request.hpp"
#include <fstream>
#include <dirent.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>

#define RES_BUF_SIZE 2048

class Response
{
	private:
		std::string								_header;

		// for header:
		std::string								_contentType;
		std::string								_contentLength;
		std::string								_statusCode;
		std::string								_reasonPhrase;
		std::string								_connection;

		std::string								_cookie;
		// std::string								_date;

		// work with file
		std::string								_fileLoc;
		std::string								_logPath;

		//flags
		bool									_logged;

		// for boby sending procces 
		long									_bytesRead;
		long									_bytesSent;
		long									_totalBytesRead;

		// for POST:
		int					pipe1[2];
		int					pipe2[2];
		pid_t				pid;
		int					ex;

	public:

		std::ifstream							_file; //поток файла из которого читает в данный момент
		std::stringstream						_stream;
		bool									_isSent; //весь ответ был выслан

		Response() : _bytesRead(0), _bytesSent(0), _totalBytesRead(0), _stream(""), _isSent(0) {};
		~Response() {};

		int				make_response_body(Request & req, const size_t id);
		void			make_response_header(Request & req, int code, std::string status, long size = 0);
		std::string		make_general_header (Request & req);
		void			make_response_html( const int code, std::string & mess, std::string loc = "" );
		void			make_response_autoidx(Request & req, std::string location, int code, std::string & status);
		void			addCookie(std::string cookie);
		// template <class T>
		// int				sendResponse(T * input, const size_t socket);
		int 			sendResponse_file(const size_t socket);
		int 			sendResponse_stream(const size_t socket);

		void			cleaner();

		int	&			getPipeWrite();
		int	&			getPipeRead();
		
		int				getContentLenght();
		std::string		getHeader();
		std::string		getResponceContType();
		std::string		getReasonPhrase();
		std::string		getFileLoc();
		std::string		getCookie();
		std::ifstream &	getFileStream();
		std::stringstream &	getStrStream();

		void			setFileLoc(std::string location);
		void			setContentType(std::string type);
		void			setLogPath(std::string path);

		//trash
		void	show_all() {
			std::cout << "_header: " << _header << "\n";
			std::cout << "_contentType: " << _contentType << "\n";
			std::cout << "_contentLength: " << _contentLength << "\n";
			std::cout << "_statusCode: " << _statusCode << "\n";
			std::cout << "_reasonPhrase: " << _reasonPhrase << "\n";
			std::cout << "_connection: " << _connection << "\n";
			std::cout << "_fileLoc: " << _fileLoc << "\n";
			std::cout << "_bytesRead: " << _bytesRead << "\n";
			std::cout << "_bytesSent: " << _bytesSent << "\n";
			std::cout << "_totalBytesRead: " << _totalBytesRead << "\n";
		}
		
		//for makePostResponse:
		void addCgiVar(char ***envp, Request & req, std::vector<std::string> & envpVector);
		char **addCgiVar(Request & req, char **envp);
		/*	adds to exported environment variables new three
			which are CGI environment variables to pass them all
			to CGI new stream */
		bool openFile();
		bool formHeaderLog(std::string str, int socket);
		int extractCgiHeader( Request & req );
		void createSubprocess( Request & req, std::string & path, char **envp);
};

#endif