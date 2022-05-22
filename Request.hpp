#ifndef RERUEST_HPP
# define RERUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <map>

// > GET / HTTP/1.1
// > Host: google.com
// > User-Agent: curl/7.54.0
// > Accept: */*

class Request {
private:
    std::string                             _method;
    std::string                             _reqURI;
    std::string                             _protocolVersion;
    std::map<std::string, std::string>      _headers;
    std::string                             _body;


public:
    Request();
    ~Request();
    void            parseText(std::string text);
    void            getStartLine(std::string str);
};



#endif