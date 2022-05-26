#ifndef RERUEST_HPP
# define RERUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <map>

class Request {
private:
	//parse data for request
	std::string                             _method;
	std::string                             _reqURI;
	std::string                             _protocolVersion;
	std::map<std::string, std::string>      _headers;
	std::string                             _body;
	std::string								_MIMEType;
	std::string								_contentType;

	//data for response
	std::string								_responseContentType;
	std::map<std::string, std::string>		_typesMIME;

public:
	Request();
	~Request();
	void            						parseText( std::string );
	void            						parseStartLine( std::string );
	void									parseMapHeaders( std::vector<std::string> , size_t );
	size_t          						parseStrBody( std::vector<std::string> );
	void									parseMIMEType();
	void									findType();

	std::string								getMethod();
	std::string								getReqURI();
	std::string								getProtocolVer();
	std::map<std::string, std::string>		getHeadears();
	std::string								getBody();
	std::string								getMIMEType();
	std::string								getContentType();
};



#endif