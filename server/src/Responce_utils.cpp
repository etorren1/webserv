#include "Response.hpp"
#include "Client.hpp"

// void    Response::autoindex() {
//     DIR *dir;
//     struct dirent *entry;

//     dir = opendir("/");
//     if (!dir) {
//         perror("diropen");
//         exit(1);
//     };

//     while ( (entry = readdir(dir)) != NULL) {
//         printf("%d - %s [%d] %d\n",
//             entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);
//     };

//     closedir(dir);
// }

#include <stdio.h>
#include <stdlib.h>
#include <stack>

void    Client::autoindex( const std::string & path ) {
    DIR *dir;
    struct dirent *entry;

    status &= ~AUTOIDX;
    dir = opendir(path.c_str());
    if (!dir) {
        statusCode = 403;
        generateErrorPage(403);
        return ;
    };
    std::stack<std::string> q;
    while ( (entry = readdir(dir)) != NULL) {
        // if (entry->d_name[0] != '.') {
            q.push(entry->d_name);
        // }
    };
    closedir(dir);

    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">" \
                                "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" \
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
                                "<title>Title</title></head><body><div class=\"container\">" \
                                "<h1>autoindex</h1><hr></hr>";
    while ( q.size() ) {
        responseBody.append("<p><a href=\"" + q.top() + "\">" + q.top() + "</a></p>");
        q.pop();
    }
    responseBody += "<hr></hr><p>webserver</p></div></body></html>";
    std::string header, location;
    
    if (statusCode == 200) {
        header = "HTTP/1.1 200 OK\r\nVersion: HTTP/1.1" \
                         "\r\nContent-Type: text/html\r\nContent-Length: " + itos(responseBody.length()) + "\r\n\r\n";
    }
    else if (statusCode == 301) {
        location = "Location: http://" + req.getHost() + req.getReqURI() + "/\r\n";
        header = "HTTP/1.1 301 Moved Permanently\r\nVersion: HTTP/1.1\r\n" + location + \
                         "\r\nContent-Type: text/html\r\nContent-Length: " + itos(responseBody.length()) + "\r\n\r\n";
    }
    std::string response = header + responseBody;
    size_t res = send(socket, response.c_str(), response.length(), 0);
    responseBody.clear();
    response.clear();
    header.clear();
    req.cleaner();
    status &= ~REQ_DONE;
}