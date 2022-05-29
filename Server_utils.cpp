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
            std::time_t result = std::time(nullptr);
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

void	Server::generateErrorPage(int error, int id) {
    std::string mess = "none";
    const int &code = error;
    std::map<int, std::string>::iterator it = resCode.begin();
    for (; it != this->resCode.end(); it++) {
        if (code == (*it).first) {
            mess = (*it).second;
        }
    }
    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Error page </title></head><body><div class=\"container\"><h2>" + itos(code) + "</h2><h3>" + mess + "</h3><p><a href=\"#homepage\">Click here</a> to redirect to homepage.</p></div></body></html>";
    std::string header = req.getProtocolVer() + " " + itos(code) + " " + mess + "\n" + "Version: " + req.getProtocolVer() + "\n" + "Content-Type: " + req.getContentType() + "\n" + "Content-Length: " + itos(responseBody.length()) + "\n\n";
    std::string response = header + responseBody;
    size_t res = send(fds[id].fd, response.c_str(), response.length(), 0);
    // std::cout << GREEN << response << RESET;
}

void Server::parseLocation() {
    if (existDir(req.getReqURI().c_str()))
        std::cout << "Directory exist\n";
    else
        std::cout << "Directory don't exist\n";
    // std::cout << srvs[req.getHost()]->lctn.at(req.getReqURI())->get_root() << "\n";
    std::cout << req.getHost() << ", " << req.getReqURI() << "\n";
    try    {
        srvs.at(req.getHost())->lctn.at(req.getReqURI())->get_root();
    }
    catch(const std::exception& e)    {
        try        {
            size_t pos = req.getHost().find(":");
            if (pos != std::string::npos) {
                std::string ip = "0.0.0.0" + req.getHost().substr(pos);
                req.setHost(ip);
            }
            srvs.at(req.getHost())->lctn.at(req.getReqURI())->get_root();
        }
        catch(const std::exception& e)        {
            
            std::cerr << e.what() << '\n';
        }
    }
}