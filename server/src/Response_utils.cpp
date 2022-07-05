#include "Response.hpp"
#include "Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stack>

void	Response::make_response_html( const int code, std::string & mess, std::string loc ) {
    std::string location;
    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"> \
<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> \
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
<title>" + itos(code) + " " + mess + "</title></head><body  align=\"center\"><div class=\"container\"><h1>" \
+ itos(code) + " " + mess + "</h1><hr></hr> \
<p> with ♡ webserver</p></div></body></html>";
    if (!loc.empty())
        location = "Location: " + loc + "\r\n";
    std::string header = "HTTP/1.1 " + itos(code) + " " + mess + "\r\n" + location + "Version: " + "HTTP/1.1" \
                         + "\r\n" + "Content-Type: " + "text/html" + "\r\n" + "Content-Length: " + itos(responseBody.length()) + "\r\n\r\n";
    std::string response = header + responseBody;
    //std::cout << YELLOW << response << "\n" << RESET;
    _contentLength = responseBody.length();
	_stream << response;
}

void	Response::make_response_autoidx(Request req, std::string path, int code, std::string & status){
    DIR *dir;
    struct dirent *entry;

    if (path.size() == 1)
        path = "." + path;
    dir = opendir(path.c_str());
    if (!dir)
        codeException(403);
    std::stack<std::string> q;
    while ( (entry = readdir(dir)) != NULL)
            q.push(entry->d_name);
    closedir(dir);
    std::string body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">" \
                                "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" \
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
                                "<title>Title</title></head><body><div class=\"container\">" \
                                "<h1>autoindex</h1><hr></hr>";
    while ( q.size() ) {
        body.append("<p><a href=\"" + q.top() + "\">" + q.top() + "</a></p>");
        q.pop();
    }
    body += "<hr></hr><p>webserver</p></div></body></html>";
    make_response_header(req, code, status, body.size());
    _stream << body;
}
