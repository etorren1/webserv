#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Server.hpp"
#include "Utils.hpp"

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
		std::string								_contentRange; //for 206

		// other
		std::string								_fileLoc;



	public:
		Response() {};
		~Response() {};

		void			make_response_body(Request req);
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

		void			setFileLoc(std::string location);
};

#endif