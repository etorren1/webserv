#include "Request.hpp"


Request::Request() {
    this->_typesMIMO.insert(std::make_pair("json", "application/json"));
    this->_typesMIMO.insert(std::make_pair("javascript", "application/javascript"));
    this->_typesMIMO.insert(std::make_pair("pdf", "application/pdf"));
    this->_typesMIMO.insert(std::make_pair("postscript", "application/postscript"));
    this->_typesMIMO.insert(std::make_pair("zip", "application/zip"));

    this->_typesMIMO.insert(std::make_pair("aac", "audio/aac"));
    this->_typesMIMO.insert(std::make_pair("mpeg", "audio/mpeg"));
    this->_typesMIMO.insert(std::make_pair("ogg", "audio/ogg"));

    this->_typesMIMO.insert(std::make_pair("gif", "image/gif"));
    this->_typesMIMO.insert(std::make_pair("jpeg", "image/jpeg"));
    this->_typesMIMO.insert(std::make_pair("pjpeg", "image/pjpeg"));
    this->_typesMIMO.insert(std::make_pair("png", "image/png"));
    this->_typesMIMO.insert(std::make_pair("tiff", "image/tiff"));
    this->_typesMIMO.insert(std::make_pair("webp", "image/webp"));

    this->_typesMIMO.insert(std::make_pair("cmd", "text/cmd"));
    this->_typesMIMO.insert(std::make_pair("css", "text/css"));
    this->_typesMIMO.insert(std::make_pair("csv", "text/csv"));
    this->_typesMIMO.insert(std::make_pair("html", "text/html"));
    this->_typesMIMO.insert(std::make_pair("plain", "text/plain"));
    this->_typesMIMO.insert(std::make_pair("php", "text/php"));
    this->_typesMIMO.insert(std::make_pair("xml", "text/xml"));

    this->_typesMIMO.insert(std::make_pair("mp4", "video/mp4"));
    this->_typesMIMO.insert(std::make_pair("quicktime", "video/quicktime"));
    this->_typesMIMO.insert(std::make_pair("webm", "video/webm"));
}

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
    parseMIMEType();
}

void Request::parseStartLine(std::string str) {
    size_t pos = str.find(" ");
    this->_method = str.substr(0, pos);
    str.erase(pos, 1);
    this->_reqURI = str.substr(pos, str.find(" ") - pos);
    size_t pos_n = str.find(" ");
    str.erase(str.find(' '), 1);
    this->_protocolVersion = str.substr(pos_n);
    // std::cout << "_method = |" << _method << "|\n";
    // std::cout << "_reqURI = |" << _reqURI << "|\n";
    // std::cout << "_protocolVersion = |" << _protocolVersion << "|\n";
}

size_t Request::parseStrBody(std::vector<std::string> vec) {
    // std::cout << "_body = |" << _body << "|\n";

    // const char *c = vec[]
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

void Request::parseMIMEType() {
    size_t pos = 0;
    size_t typeEnd = _reqURI.find("?");
    if (_reqURI.length() > 1) {
        pos = _reqURI.find(".");
        if (pos != std::string::npos) {
            if (typeEnd != std::string::npos)
                _MIMEType = _reqURI.substr(pos + 1, typeEnd - pos - 1);
            else
                _MIMEType = _reqURI.substr(pos + 1);
        }
    }
    // std::cout << "_MIMEType = |" << _MIMEType << "|\n";
}

std::string Request::getMethod() { return this->_method; }
std::string Request::getReqURI() { return this->_reqURI; }
std::string Request::getProtocolVer() { return this->_protocolVersion; }
std::map<std::string, std::string> Request::getHeadears() { return this->_headers; }
std::string Request::getBody() { return this->_body; }
std::string Request::getMIMEType() { return this->_MIMEType; }
std::string Request::getContentType() { _contentType = "text"; return this->_contentType; }