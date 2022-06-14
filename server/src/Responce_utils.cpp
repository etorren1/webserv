#include "Response.hpp"
#include "Client.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stack>

void	Response::make_response_error( const int error, std::string & mess ) {

    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"> \
                                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> \
                                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
                                <title>" + itos(error) + " " + mess + "</title></head><body  align=\"center\"><div class=\"container\"><h1>" \
                                + itos(error) + " " + mess + "</h1><hr></hr> \
                                <p>webserver</p></div></body></html>";
    std::string header = "HTTP/1.1 " + itos(error) + " " + mess + "\n" + "Version: " + "HTTP/1.1" \
                         + "\n" + "Content-Type: " + "text/html" + "\n" + "Content-Length: " + itos(responseBody.length()) + "\n\n";
    std::string response = header + responseBody;
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
