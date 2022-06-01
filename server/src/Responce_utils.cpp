#include "Response.hpp"
#include "Client.hpp"
#include <dirent.h>

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

struct s_cont
{
    std::string name;
    bool        is_dir;
} t_cont;


void    Client::autoindex( const std::string & path ) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path.c_str());
    if (!dir) {
        generateErrorPage(403);
        return ;
    };
    std::stack<struct s_cont *> q;
    while ( (entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            struct s_cont *s = new struct s_cont;
            s->name = entry->d_name;
            entry->d_type == 4 ? s->is_dir = true : s->is_dir = false;
            q.push(s);
        }
    };
    closedir(dir);

    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">" \
                                "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" \
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
                                "<title>Title</title></head><body><div class=\"container\">" \
                                "<h1>Directoru</h1><hr></hr>";
    while ( q.size() ) {
        struct s_cont *s = q.top();
        if (s->is_dir)
            responseBody.append("<p><a href=\"" + path + "/" + s->name + "\">" + s->name + "</a></p>");
        else 
            responseBody.append("<p>" + s->name + "</p>");
        delete s;
        q.pop();
    }
    responseBody += "<hr></hr><p>webserver</p></div></body></html>";
    std::string header = "HTTP/1.1 200 OK\r\nVersion: HTTP/1.1" \
                         "\r\nContent-Type: text/html\r\nContent-Length: " + itos(responseBody.length()) + "\r\n\r\n";
    std::string response = header + responseBody;
    size_t res = send(socket, response.c_str(), response.length(), 0);
    responseBody.clear();
    response.clear();
    header.clear();
}