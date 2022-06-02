#ifndef RERUEST_HPP
# define RERUEST_HPP

# include <iostream>
# include <sstream>
# include <unistd.h>
// # include <string>
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
	std::string                             _body;
	std::string								_MIMEType;
	std::string								_host;
	std::vector<std::string>				_dirs;
	bool									_file;
	// std::string								_fileName;

	//data for response
	std::string								_responseContentType;

public:
	Request();
	~Request();
	void            						parseText( std::string );
	void            						parseStartLine( std::string );
	void									parseMapHeaders( std::vector<std::string> , size_t );
	size_t          						parseStrBody( std::vector<std::string> );
	void									parseMIMEType();
	void									findType();
	void									findHost();
	void									splitDirectories();
	void									cleaner();
	void									setMIMEType(std::string);

	std::string								getDirNamesWithoutRoot(std::string);
	std::string								getMethod() const;
	std::string								getReqURI() const;
	std::string								getProtocolVer() const;
	std::map<std::string, std::string>		getHeadears() const;
	std::string								getBody() const;
	std::string								getMIMEType() const;
	std::string								getContentType() const;
	std::string								getHost() const;
	std::vector<std::string>				getDirs() const;

	// bool									isFile();
	
	void									setHost(std::string);
};



#endif
