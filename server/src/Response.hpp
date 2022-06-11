#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Utils.hpp"
#include "Request.hpp"
#include <fstream>
#include <dirent.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>

#define RES_BUF_SIZE 2048
// #define STREAM_IS_FILE 0
// #define STREAM_IS_STR 1

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

		// std::string								_date;

		// work with file
		std::string								_fileLoc;

		//flags

		// for boby sending procces 
		long									_bytesRead;
		long									_bytesSent;
		long									_totalBytesRead;

	public:
		std::ifstream							_file; //поток файла из которого читает в данный момент
		std::stringstream						_stream;
		bool									_isSent; //весь ответ был выслан

		Response() : _bytesRead(0), _bytesSent(0), _totalBytesRead(0), _isSent(0) {};
		~Response() {};

		int				make_response_body(Request req, const size_t id);
		void			make_response_header(Request req, int code, std::string status, long size = 0);
		std::string		make_general_header (Request req, int statusCode);
		void			make_response_error( const int error, std::string & mess );
		void			make_response_autoidx(Request req, std::string location, int code, std::string & status);
		// template <class T>
		// int				sendResponse(T * input, const size_t socket);
		int sendResponse_file(const size_t socket);
		int sendResponse_stream(const size_t socket);

		void			cleaner();

		std::string		getHeader();
		std::string		getContentType();
		std::string		getStatusCode();
		std::string		getReasonPhrase();
		std::string		getFileLoc();
		// std::ifstream 	getFileStream();
		std::stringstream &	getStrStream();

		void			setFileLoc(std::string location);
		void			setContentType(std::string type);
		void			setStatusCode(std::string code);							//убрать
		// void			setInput(std::ifstream &_file);
		// void			setStrStream(std::stringstream stream);

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
		void addCgiVar(char ***envp, Request req);
		/*	adds to exported environment variables new three
			which are CGI environment variables to pass them all
			to CGI new stream */
		void openFile();
};

#endif