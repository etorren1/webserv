#include "Errors.hpp"

Errors::Errors(){
    this->errMap.insert(std::make_pair(200, "OK"));
    this->errMap.insert(std::make_pair(201, "Created"));
    this->errMap.insert(std::make_pair(204, "No Content"));
    this->errMap.insert(std::make_pair(304, "Not Modified"));
    this->errMap.insert(std::make_pair(400, "Bad Request"));
    this->errMap.insert(std::make_pair(401, "Unauthorized"));
    this->errMap.insert(std::make_pair(402, "Payment Required"));
    this->errMap.insert(std::make_pair(403, "Forbidden"));
    this->errMap.insert(std::make_pair(404, "Not Found"));
    this->errMap.insert(std::make_pair(405, "Method Not Allowed"));
    this->errMap.insert(std::make_pair(406, "Not Acceptable"));
    this->errMap.insert(std::make_pair(407, "Proxy Authentication Required"));
    this->errMap.insert(std::make_pair(408, "Request Timeout"));
    this->errMap.insert(std::make_pair(409, "Conflict"));
    this->errMap.insert(std::make_pair(500, "Internal Server Error"));
    this->errMap.insert(std::make_pair(501, "Not Implemented"));
    this->errMap.insert(std::make_pair(502, "Bad Gateway"));
    this->errMap.insert(std::make_pair(503, "Service Unavailable"));
    this->errMap.insert(std::make_pair(504, "Gateway Timeout"));
}

Errors::~Errors(){}

void Errors::findErrorCode( std::string body, int code) {
    
}