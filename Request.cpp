#include "Request.hpp"


Request::Request() {
    this->_typesMIME.insert(std::make_pair("json", "application/json"));
    this->_typesMIME.insert(std::make_pair("javascript", "application/javascript"));
    this->_typesMIME.insert(std::make_pair("pdf", "application/pdf"));
    this->_typesMIME.insert(std::make_pair("postscript", "application/postscript"));
    this->_typesMIME.insert(std::make_pair("zip", "application/zip"));
    this->_typesMIME.insert(std::make_pair("xml", "application/xml"));

    this->_typesMIME.insert(std::make_pair("aac", "audio/aac"));
    this->_typesMIME.insert(std::make_pair("mpeg", "audio/mpeg"));
    this->_typesMIME.insert(std::make_pair("ogg", "audio/ogg"));

    this->_typesMIME.insert(std::make_pair("gif", "image/gif"));
    this->_typesMIME.insert(std::make_pair("jpeg", "image/jpeg"));
    this->_typesMIME.insert(std::make_pair("pjpeg", "image/pjpeg"));
    this->_typesMIME.insert(std::make_pair("png", "image/png"));
    this->_typesMIME.insert(std::make_pair("tiff", "image/tiff"));
    this->_typesMIME.insert(std::make_pair("ico", "image/vnd.microsoft.icon")); //"image/x-icon"
    this->_typesMIME.insert(std::make_pair("webp", "image/webp"));

    this->_typesMIME.insert(std::make_pair("cmd", "text/cmd"));
    this->_typesMIME.insert(std::make_pair("css", "text/css"));
    this->_typesMIME.insert(std::make_pair("csv", "text/csv"));
    this->_typesMIME.insert(std::make_pair("html", "text/html"));
    this->_typesMIME.insert(std::make_pair("plain", "text/plain"));
    this->_typesMIME.insert(std::make_pair("php", "text/php"));

    this->_typesMIME.insert(std::make_pair("mp4", "video/mp4"));
    this->_typesMIME.insert(std::make_pair("quicktime", "video/quicktime"));
    this->_typesMIME.insert(std::make_pair("webm", "video/webm"));
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
        for (size_t i = 0; i < vec.size(); i++) {
            // std::cout << "vec[" << i << "].length() - " << vec[i].length() << "\n";
            if (vec[i].length() == 0 || (i == vec.size() - 1 && vec[i].length() != 0))
                pos = vec.size() - 1;
        }
    }
    // std::cout << "pos = " << pos << "\n";
    parseMapHeaders(vec, pos);
    parseMIMEType();
    if (_reqURI.length() > 1)
        splitDirectories();
    else
        _dirs.push_back(getReqURI());
    findType();
    findHost();
}

void Request::parseStartLine(std::string str) {
    size_t pos = str.find(" ");
    this->_method = str.substr(0, pos);
    str.erase(pos, 1);
    this->_reqURI = str.substr(pos, str.find(" ") - pos);
    size_t pos_n = str.find(" ");
    str.erase(str.find(' '), 1);
    this->_protocolVersion = str.substr(pos_n);
    // std::cout << GREEN << "_method = |" << _method << "|\n";
    // std::cout << GREEN << "_reqURI = |" << _reqURI << "|\n";
    // std::cout << GREEN << "_protocolVersion = |" << _protocolVersion << "|\n";
}

size_t Request::parseStrBody(std::vector<std::string> vec) {
    // std::cout << GREEN << "_body = |" << _body << "|\n";

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
        // std::cout << "vec[i] = " << vec[i] << "\n";
        n = vec[i].find(":");
        key = vec[i].substr(0, n);
        val = vec[i].substr(n + 2);
        _headers.insert(std::make_pair(key, val));
        // std::cout << "[" << key << "] - [" << val << "]\n";
    }
    // std::cout << "_headers.size() - " << _headers.size() << "\n";
    // std::map<std::string, std::string>::iterator it = _headers.begin();
    // for (; it != _headers.end(); it++) {
    //     std::cout << "|" << (*it).first << "| - |" << (*it).second << "|\n";
    //     // it++;
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
        } else {
            _MIMEType = "none";
        }
    }
    // std::cout << GREEN << "_MIMEType = |" << _MIMEType << "|\n";
}

void Request::findType() {
    std::map<std::string, std::string>::iterator it = _typesMIME.begin();
    for ( ; it != _typesMIME.end(); it++) {
        if (_MIMEType == (*it).first) {
            _responseContentType = (*it).second;
        }
    }
    // std::cout << GREEN << "_responseContentType = |" << _responseContentType << "|\n";
}

void Request::findHost() {
    std::map<std::string, std::string>::iterator it = _headers.begin();
    for ( ; it != _headers.end(); it++) {
        if ((*it).first == "Host")
            _host = (*it).second;
    }
    size_t pos = _host.find("localhost");
    if (pos != std::string::npos) {
        std::string ip = "127.0.0.1" + _host.substr(9);
        _host = ip;
    }
    // std::cout << GREEN << "_host = |" << _host << "|\n";
}

void Request::splitDirectories() {
    std::string str;
    _dirs.push_back(_reqURI);
    // std::cout << "_dirs[0] = " << _dirs[0] << "\n";
    size_t pos = _reqURI.find_last_of("/");
    str = _reqURI.substr(0, pos);
    _dirs.push_back(str);
    for (size_t i = 1; i < _reqURI.length() - 1; i++) {
        pos = str.find_last_of("/");
        if (pos != std::string::npos && pos != 0) {
            str = str.substr(0, pos);
            _dirs.push_back(str);
        }
    }
    _dirs.push_back("/");
    // std::vector<std::string>::iterator it = _dirs.begin();
    // for (; it != _dirs.end(); it++)
    //     std::cout << GREEN << "it = " << *it << RESET << "\n";
}

void Request::cleaner() {
    _method.clear();
    _reqURI.clear();
    _protocolVersion.clear();
    _headers.clear();
    _body.clear();
    _MIMEType.clear();
    _responseContentType.clear();
    _host.clear();
    _dirs.clear();
}

std::string Request::getMethod() const { return this->_method; }
std::string Request::getReqURI() const { return this->_reqURI; }
std::string Request::getProtocolVer() const { return this->_protocolVersion; }
std::map<std::string, std::string> Request::getHeadears() const { return this->_headers; }
std::string Request::getBody() const { return this->_body; }
std::string Request::getMIMEType() const { return this->_MIMEType; }
std::string Request::getContentType() const { return this->_responseContentType; }
std::string Request::getHost() const { return this->_host; }
std::vector<std::string> Request::getDirs() const { return this->_dirs; }

void Request::setHost(std::string host) { _host = host; }