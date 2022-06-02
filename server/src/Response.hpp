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

class Response
{
	private:
		std::string								_header;
		std::string								_body;

		// for header:
		std::string								_contentType;
		std::string								_contentLength;
		std::string								_statusCode;
		std::string								_reasonPhrase;
		std::string								_connection;

		// std::string								_date;

		// other
		std::string								_fileLoc;
		int										_sendingFinished;
		std::ifstream							_input; //поток файла из которого читает в данный момент
		size_t									_range_begin;

		// for boby sending procces 
		long									_bytesRead;
		long									_bytesSent;
		long									_totalBytesRead;

		int 									count;

		char 			buffer[RES_BUF_SIZE];
	public:
		int										_hederHasSent; //сделать геттер и сеттер

		Response() : _sendingFinished(0), _bytesRead(0), _bytesSent(0), _totalBytesRead(0),\
					_hederHasSent(0), _range_begin(0), count(0) {};
		~Response() {};

		int				make_response_body(Request req, const size_t id);
		void			make_response_header(Request req);
		std::string		make_general_header (Request req, std::string response_body);
		std::string		find_requested_file_path(Request req);
		std::string		parse_uri(std::string uri);
		void			clearResponseObj();

		std::string		getHeader();
		std::string		getBody();
		std::string		getContentType();
		std::string		getStatusCode();
		std::string		getReasonPhrase();
		std::string		getFileLoc();

		void			setFileLoc(std::string location);
		void			setContentType(std::string type);
};

#endif