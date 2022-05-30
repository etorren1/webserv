#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Utils.hpp"
#include "Request.hpp"
#include <poll.h>

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
		long long								_readFrom;

	public:
		Response() : _sendingFinished(0), _readFrom(0) {};
		~Response() {};

		void			make_response_body(Request req, const size_t id, std::vector<struct pollfd> fds);
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