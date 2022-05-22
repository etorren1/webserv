#ifndef RERUEST_HPP
# define RERUEST_HPP

# include "iostream"
# include <vector>

// > GET / HTTP/1.1
// > Host: google.com
// > User-Agent: curl/7.54.0
// > Accept: */*

class Request {
private:
    std::string                             _method;
    std::string                             _originForm; /*Абсолютный путь, за которым следует '?' и строка запроса. Это самая распространённая форма, называемая исходной формой (origin form) . Используется с методами GET, POST*/
    std::string                             _absoluteForm; /*Полный URL - абсолютная форма (absolute form) , обычно используется с GET при подключении к прокси.
    GET http://developer.mozilla.org/ru/docs/Web/HTTP/Messages HTTP/1.1 */
    std::string                             _protocolVersion;
    std::vector<std::string, std::string>   _headers;
    std::string                             _body;


public:
    Request(std::string text);
    ~Request();
};

Request::Request(std::string text) {
}

Request::~Request() {
}


#endif