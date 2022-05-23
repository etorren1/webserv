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
    // std::cout << "vec.size() = " << vec.size() << "\n";
    size_t pos = this->parseStrBody(vec);
    if (pos == 0) {
        for (size_t i = 0; i < vec.size() - 1; i++) {
            if (vec[i].length() == 0)
                pos = vec.size() - 1;
        }
    }
    // std::cout << "pos = " << pos << "\n";
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
    // std::cout << "_method = |" << _method << "\n";
    // std::cout << "_reqURI = |" << _reqURI << "\n";
    // std::cout << "_protocolVersion = |" << _protocolVersion << "\n";
}

size_t Request::parseStrBody(std::vector<std::string> vec) {
    // std::cout << "body = |" << _body << "|\n";

    // const char *c = vec[]
    size_t pos = 0;
    for (size_t i = 0; i < vec.size() - 1; i++) {
        if (vec[i].length() == 0) {
            std::cout << "i = " << i << ", len = " << vec[i].length() << "\n";
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
        val = vec[i].substr(n + 1);
        _headers.insert(std::make_pair(key, val));
        std::cout << "[" << key << "] - [" << val << "]\n";
    }
    // std::map<std::string, std::string>::iterator it = _headers.begin();
    // for (size_t i = 0; i < _headers.size() - 1; i++) {
    //     std::cout << "|" << (*it).first << "| - |" << (*it).second << "|\n";
    //     it++;
    // }
}

std::string Request::getMethod() { return this->_method; }
std::string Request::getReqURI() { return this->_reqURI; }
std::string Request::getProtocolVer() { return this->_protocolVersion; }
std::map<std::string, std::string> Request::getHeadears() { return this->_headers; }
std::string Request::getBody() { return this->_body; }