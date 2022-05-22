#include "Request.hpp"


Request::Request() {
}

Request::~Request() {
}

void Request::parseText(std::string text) {
    std::vector<std::string> vec;
    std::istringstream strs(text);
    std::string s;
    while (std::getline(strs, s, '\n'))
        vec.push_back(s);
    this->getStartLine(vec[0]);
    // std::cout << vec.size() << "\n";
    // std::vector<std::string>::iterator it = vec.begin();
    // for (; it != vec.end(); it++)
    //     std::cout << *it << "\n";


}

void Request::getStartLine(std::string str) {
    size_t pos = str.find(" ");
    this->_method = str.substr(0, pos);
    str.erase(pos, 1);
    this->_reqURI = str.substr(pos, str.find(" ") - pos);
    size_t pos_n = str.find(" ");
    str.erase(str.find(' '), 1);
    this->_protocolVersion = str.substr(pos_n);
}