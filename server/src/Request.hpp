#ifndef RERUEST_HPP
# define RERUEST_HPP

# include <iostream>
# include <sstream>
# include <unistd.h>
# include <cstring>
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
	std::string								_MIMEType;
	std::string								_host;
	std::string								_raw_host;
	std::string								_contentLength;
	std::string								_contentType;
	std::string								_transferEnc;
	std::vector<std::string>				_dirs;
	bool									_bodyExist;
	std::string								_cgiStatusCode;
	std::string								_boundary;
	std::string								_referer;

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
	void									findBoundary();
	void									findHost();
	void									splitDirectories();
	void									cleaner();
	void									setMIMEType( std::string );									
	int										checkHeaders( std::map<std::string, std::string> fMap, std::string checked, std::string &header);
	void									parseBody( std::stringstream &, size_t &, std::vector<std::string>&);
	void									splitLocation( std::string );
	void 									parseEnvpFromBody( std::stringstream &, std::vector<std::string>& );
	void									clearHeaders();

	std::string								getMethod() const;
	std::string								getReqURI() const;
	std::string								getProtocolVer() const;
	std::map<std::string, std::string>	const& getHeadears() const;
	std::string								getMIMEType() const;
	std::string								getResponceContType() const;
	std::string								getHost() const;
	std::string								getRawHost() const;
	std::string								getContentLenght() const;
	std::string								getContType() const;
	std::string								getTransferEnc() const;
	std::vector<std::string>				getDirs() const;
	int										getReqSize() const;
	std::string								getCgiStatusCode() const;
	std::string								getBoundary() const;
	std::string								getReferer() const;

	void					trimBoundary( std::stringstream & reader, size_t size );
	void					trimChunks( std::stringstream & reader, size_t size );

	void									setContType(std::string);
	void									setHost(std::string);
	void									setReqURI(std::string);
};



#endif
