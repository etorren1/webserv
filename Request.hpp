#ifndef RERUEST_HPP
# define RERUEST_HPP

# include "iostream"
# include <vector>

class Request
{
private:
    std::string _method;
    std::string _originForm;
    std::string _absoluteForm;
    std::string _protocolVersion;
    std::vector<std::string, std::string> _headers;

public:
    Request(/* args */);
    ~Request();
};

Request::Request(/* args */)
{
}

Request::~Request()
{
}


#endif