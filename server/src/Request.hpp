#ifndef RERUEST_HPP
# define RERUEST_HPP

# include <iostream>
# include <sstream>
# include <unistd.h>
# include <vector>
# include <map>
# include "Utils.hpp"

class Request {
private:
	//parse data for request
	std::map<std::string, std::string>		_typesMIME;
	
	std::string                             _method;
	std::string                             _reqURI;
	std::string                             _protocolVersion;
	std::map<std::string, std::string>      _headers;
	// std::string                             _body;
	std::string								_MIMEType;
	std::string								_host;
	std::string								_contentLenght;
	std::string								_contentType;
	std::string								_transferEnc;
	std::vector<std::string>				_dirs;
	std::vector<char>						_body;
	int										_reqSize;
	bool									_bodyExist;

	//data for response
	std::string								_responseContentType;

public:
	Request();
	~Request();
	bool            						parseText( std::string );
	void            						parseStartLine( std::string );
	void									parseMapHeaders( std::vector<std::string> , size_t );
	size_t          						findHeadersEnd( std::vector<std::string> );
	void									parseMIMEType();
	// void									findType();
	void									findHost();
	void									splitDirectories();
	void									cleaner();
	void									setMIMEType(std::string);									
	int										checkHeaders(std::map<std::string, std::string> fMap, std::string checked, std::string &header);
	void									parseBody(std::string );
	void									splitLocation(std::string);


	std::string								getMethod() const;
	std::string								getReqURI() const;
	std::string								getProtocolVer() const;
	std::map<std::string, std::string>		getHeadears() const;
	std::vector<char>						getBody() const;
	std::string								getMIMEType() const;
	std::string								getContentType() const;
	std::string								getHost() const;
	std::string								getСontentLenght() const;
	std::string								getСontType() const;
	std::string								getTransferEnc() const;
	std::vector<std::string>				getDirs() const;
	int										getReqSize() const;

	void									setHost(std::string);
	void									setReqSize();
	void									setReqURI(std::string);
};



#endif
