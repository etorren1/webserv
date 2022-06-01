#include "Server.hpp"

bool    Server::isServerSocket( const int & fd ) {
    if (srvSockets.find(fd) != srvSockets.end())
        return true;
    return false;
}

static void    rek_mkdir( std::string path)
{
    int sep = path.find_last_of("/");
    std::string create = path;
    if (sep != std::string::npos) {
        rek_mkdir(path.substr(0, sep));
        path.erase(0, sep);
    }
    mkdir(create.c_str(), 0777);
}

void    Server::writeLog( const std::string & path, const std::string & header, const std::string & text ) {
    if (path != "off") {
        int fd;
        char buf[BUF_SIZE];
        fd = open(path.c_str(), O_RDWR | O_CREAT , 0777); // | O_TRUNC
        if (fd < 0) {
            int sep = path.find_last_of("/");
            if (sep != std::string::npos) {
                rek_mkdir(path.substr(0, sep));
            }
            fd = open(path.c_str(), O_RDWR | O_CREAT , 0777); // | O_TRUNC
            if (fd < 0) {
                std::cerr << RED << "Error: can not open or create log file" << RESET << "\n";
                return ;
            }
            else
                while (read(fd, buf, BUF_SIZE) > 0) {}
        }
        else
            while (read(fd, buf, BUF_SIZE) > 0) {}
        if (fd) {
            std::time_t result = std::time(NULL);
            std::string time = std::asctime(std::localtime(&result));
            time = "[" + time.erase(time.size() - 1) + "] ";
            write(fd, time.c_str(), time.size());
            write(fd, header.c_str(), header.size());
            write(fd, "\n\n", 2);
            write(fd, text.c_str(), text.size());
            write(fd, "\n\n", 2);
            close (fd);
        }
    }
}

int     Server::checkBodySize( const size_t socket, const std::string & text ) {
    size_t bodySize;
    size_t pos = text.find("\r\n\r\n");
    if (pos != std::string::npos)
        bodySize = text.size() - pos - 4;
    if (bodySize > client[socket]->getMaxBodySize() ) {
        client[socket]->generateErrorPage(400);
        return (1);
    }
    return (0);
}