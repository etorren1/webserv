#include "Request.hpp"


Request::Request() {}

Request::~Request() {}

void Request::parseText(std::string text) {
    std::vector<std::string> vec;
    std::istringstream strs(text);
    std::string s;
    std::vector<std::string>::size_type posN = 0;
    while (std::getline(strs, s, '\n'))
        vec.push_back(s);
    this->parseStartLine(vec[0]);
    vec.erase(vec.begin());
    size_t pos = this->parseStrBody(vec);
    parseMapHeaders(vec, pos);
}

void Request::parseStartLine(std::string str) {
    size_t pos = str.find(" ");
    this->_method = str.substr(0, pos);
    str.erase(pos, 1);
    this->_reqURI = str.substr(pos, str.find(" ") - pos);
    size_t pos_n = str.find(" ");
    str.erase(str.find(' '), 1);
    this->_protocolVersion = str.substr(pos_n);
}

size_t Request::parseStrBody(std::vector<std::string> vec) {
    // std::cout << "body = |" << _body << "|\n";
    size_t pos = 0;
    for (size_t i = 0; i < vec.size() - 1; i++) {
        if (vec[i].length() == 0) {
            pos = i;
            for (; i < vec.size(); i++) {
                this->_body += vec[i];
            }
        }
    }
    return pos;
}

void Request::parseMapHeaders(std::vector<std::string> vec, size_t pos) {
    std::string key = "none";
    std::string val = "none";
    size_t n = 0;
    for (size_t i = 0; i < pos; i++) {
        n = vec[i].find(":");
        key = vec[i].substr(0, n);
        val = vec[i].substr(n + 2);
        _headers.insert(std::make_pair(key, val));
    }
}

std::string Request::getMethod() { return this->_method; }
std::string Request::getReqURI() { return this->_reqURI; }
std::string Request::getProtocolVer() { return this->_protocolVersion; }
std::map<std::string, std::string> Request::getHeadears() { return this->_headers; }
std::string Request::getBody() { return this->_body; }