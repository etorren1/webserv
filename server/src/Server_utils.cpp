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
        fd = open(path.c_str(), O_RDWR | O_CREAT | O_APPEND , 0777); // | O_TRUNC
        if (fd < 0) {
            int sep = path.find_last_of("/");
            if (sep != std::string::npos) {
                rek_mkdir(path.substr(0, sep));
            }
            fd = open(path.c_str(), O_RDWR | O_CREAT | O_APPEND, 0777); // | O_TRUNC
            if (fd < 0) {
                std::cerr << RED << "Error: can not open or create log file" << RESET << "\n";
                return ;
            }
        }
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
    size_t bodySize = 0;
    size_t pos = text.find("\r\n\r\n");
    if (pos != std::string::npos)
        bodySize = text.size() - pos - 4;
    if (bodySize > client[socket]->getMaxBodySize() ) {
        client[socket]->generateErrorPage(400);
        return (1);
    }
    return (0);
}

Server_block * Server::getServerBlock( std::string host ) const {

    Server_block * srv;
    size_t pos;
    if ((pos = host.find("localhost")) != std::string::npos)
        host = "127.0.0.1" + host.substr(9);
    try {
        srv = srvs.at(host);
    } catch(const std::exception& e) {
        try {
            pos = host.find(":");
            host = "0.0.0.0" + host.substr(pos);
            srv = srvs.at(host);
        } catch(const std::exception& e) {
            return (NULL);
        }
    }
    return srv; 
}

// ПЕРЕНЕСТИ В CLIENT
// void Server::parseLocation() {
//     std::cout << "MIME type: " << req.getMIMEType() << "\n";
//     if (req.getMIMEType().empty())// || req.getMIMEType() == "none")
//         reqType = 0; // dir
//     else
//         reqType = 1; //file
//     try    {
//         srvs.at(req.getHost())->lctn.at(req.getReqURI())->show_all();
//     }
//     catch(const std::exception& e)    {
//         try        {
//             size_t pos = req.getHost().find(":");
//             if (pos != std::string::npos) {
//                 std::string ip = "0.0.0.0" + req.getHost().substr(pos);
//                 req.setHost(ip);
//             }
//             std::vector<std::string> vec = req.getDirs();
//             std::string tmp, tmpDefPage;
//             std::string defPage = "index.html";
//             location = "/";
//             for (size_t i = 0; i < vec.size(); i++) {
//                 try {
//                     tmp = srvs.at(req.getHost())->lctn.at(vec[i])->get_root();
//                     tmpDefPage = srvs.at(req.getHost())->lctn.at(vec[i])->get_default_page();
//                     if (tmp.length() > location.length()) {
//                         location = tmp;
//                         defPage = tmpDefPage;
//                     }
//                 }
//                 catch(std::exception &e) { std::cout << "\n"; }
//             }
//                 location += vec[0].substr(1);
//                 if (reqType == 0) {
//                     if (existDir(location.c_str())) {
//                         int ret = open(location.c_str(), O_RDONLY);
//                         // struct stat s;
//                         if (!access(location.c_str(), 4)) {
//                             location += defPage;
//                             std::cout << "default_page: " <<defPage << "\n";
//                             FILE *file;
//                             file = fopen(location.c_str(), "r");
//                             if (file != NULL) {
//                                 std::cout << "File " << location << " found\n";
//                         //make_response ???
//                             } else {
//                                 throw(codeException(404));
//                             }
//                                 // FILE *file;
//                                 // file = fopen("index.html", "r");
//                                 // if (file != NULL) {
//                                 //     //make_response ???
//                                 // } else {
//                                 //     throw(codeException(404));
//                                 // }
//                             }
//                         else {
//                             std::cout << "Permission denied\n";
//                             throw(codeException(403));
//                         }
//                     } else {
//                         std::cout << "Dir " << location << " don't exist\n";
//                         throw(codeException(400));
//                     }
//                 } else {
//                     FILE *file;
//                     file = fopen(location.c_str(), "r");
//                     if (file != NULL) {
//                         std::cout << "File " << location << " found \n";
//                         //make_response ???
//                     } else {
//                         throw(codeException(404));
//                     }
//                 }
//             // srvs.at(req.getHost())->lctn.at(req.getReqURI())->show_all();
//         }
//         catch(const codeException& e)        {
//             throw(codeException(400));
//         }
//         catch(const std::exception& e) {
//             std::cerr << e.what() << '\n';
//         }
//     }
// }
