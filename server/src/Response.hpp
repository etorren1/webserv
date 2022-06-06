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
#define STREAM_IS_FILE 0
#define STREAM_IS_STR 1

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
		std::ifstream							_file; //поток файла из которого читает в данный момент
		std::stringstream						_stream;
		std::string								_fileLoc;

		//flags
		int										_streamType;

		// for boby sending procces 
		long									_bytesRead;
		long									_bytesSent;
		long									_totalBytesRead;
		char 			buffer[RES_BUF_SIZE];


		public:


		int										_hederHasSent; //сделать геттер и сеттер

		Response() : _bytesRead(0), _bytesSent(0), _totalBytesRead(0),\
					_hederHasSent(0), _streamType(0) {};
		~Response() {};

		int				make_response_body(Request req, const size_t id);
		void			make_response_header(Request req);
		std::string		make_general_header (Request req);
		// std::string		find_requested_file_path(Request req);
		// std::string		parse_uri(std::string uri);
		void			clearResponseObj();
		template <class T>
		int				sendResponse(T input, const size_t socket);



		std::string		getHeader();
		std::string		getContentType();
		std::string		getStatusCode();
		std::string		getReasonPhrase();
		std::string		getFileLoc();
		std::ifstream &	getInput();

		void			setFileLoc(std::string location);
		void			setContentType(std::string type);
		// void			setInput(std::ifstream &_file);

		//for makePostResponse:
		void addCgiVar(char ***envp, Request req);
		/*	adds to exported environment variables new three
			which are CGI environment variables to pass them all
			to CGI new stream */
		void openFile();
};

#endif