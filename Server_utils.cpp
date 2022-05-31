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
    std::cout << YELLOW << "req.getMIMEType() - " << req.getMIMEType() << "\n" << RESET;
    if (req.getMIMEType().empty() || req.getMIMEType() == "none" || req.isFile() == false)
    // if ()
        reqType = 0; // dir
    else
        reqType = 1; //file
    std::string tmp, tmpDefPage;
    Server_block *srv;
    try    {
        srv = srvs.at(req.getHost());
    }
    catch(const std::exception& e) {
        try {
            size_t pos = req.getHost().find(":");
            if (pos != std::string::npos) {
                std::string ip = "0.0.0.0" + req.getHost().substr(pos);
                req.setHost(ip);
            }
            srv = srvs.at(req.getHost());
        }
        catch(const codeException& e) {
            std::cout << "Not a server!\n";
            std::cerr << e.what() << '\n';
            return ;
            // throw(codeException(400));
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
            return ;
        }
    }



            std::vector<std::string> vec = req.getDirs();
            std::string defPage = "index.html";
            std::string desiredPath = "/";
            location = "/";
            for (size_t i = 0; i < vec.size(); i++) {
                std::cout << "vec[" << i << "] = " << vec[i] << "\n";
                try {

                    tmp = srv->lctn.at(vec[i])->get_root();
                    tmpDefPage = srv->lctn.at(vec[i])->get_default_page();
                    // std::cout << RED << "if (vec[i] = " << vec[i] << " > (desiredPath = " << desiredPath << ")\n" << RESET;
                    // std::cout << "      vec[i].length() = " << vec[i].length() << ", desiredPath.length() = " << desiredPath.length() << "\n";
                    if (vec[i].length() >= desiredPath.length()) {
                        std::cout << "tmp = " << tmp << ", tmpDefPage = " << tmpDefPage << "\n";
                        std::cout << "      desiredPath = " << desiredPath << ", vec[i] = " << vec[i] << "\n";
                        desiredPath = vec[i];
                        location = tmp;
                        defPage = tmpDefPage;
                        std::cout << "root = " << location << ", defPage = " << defPage << "\n";
                    }
                    // if (tmp.length() > location.length()) {
                    // }
                }
                catch(std::exception &e) { std::cout << "\n"; }
            }
            std::cout << "\nbefore getDirNamesWithoutRoot - " << location << "\n";
            req.getDirNamesWithoutRoot(location);
            std::cout << "\nfter getDirNamesWithoutRoot - " << location << "\n";

            location += vec[0].substr(1);
            std::cout << "\n\nlocation after += vec[0] - " << location << "\n";
            if (reqType == 0) {
                std::cout << "if path - dir\n";
                // std::cout << RED << "existDir - " << existDir(location.c_str()) << "\n" << RESET;
                // if (existDir(location.c_str())) {
                if (existDir(location.c_str())) {
                    // std::cout << "if existDir\n";
                    int ret = open(location.c_str(), O_RDONLY);
                    // struct stat s;
                    if (!access(location.c_str(), 4)) {
                        std::cout << "location without defPage - " << location << "\n";
                        if (location.back() != '/')
                            location.push_back('/');
                        location += defPage;
                        std::cout << "location with defPage - " << location << "\n";
                        std::ifstream ifile;
                        ifile.open(location.c_str());
                        if (ifile) {
                            std::cout << "file exist\n";
                        } else 
                            std::cout << "file doesn't exist\n";

                        FILE *file;
                        file = fopen(location.c_str(), "r");
                        if (file != NULL) {
                            std::cout << "File " << location << " found\n";
                    //make_response ???
                        } else {
                            throw(codeException(404));
                        }
                            // FILE *file;
                            // file = fopen("index.html", "r");
                            // if (file != NULL) {
                            //     //make_response ???
                            // } else {
                            //     throw(codeException(404));
                            // }
                        }
                    else {
                        std::cout << "Permission denied\n";
                        throw(codeException(403));
                        return ;
                    }
                } else {
                    std::cout << "Dir " << location << " doesn't exist\n";
                    throw(codeException(404));
                    return ;
                }
            } else {
                std::cout << "if " << location << " is file\n";
                FILE *file;
                std::cout << "location = " << location << "\n";
                file = fopen(location.c_str(), "r");
                std::cout << "if location can't open = " << location << "\n";
                if (file != NULL) {
                    std::cout << "File " << location << " found\n";
                    //make_response ???
                } else {
                    std::cout << "file == NULL\n";
                    throw(codeException(404));
                    return;
                }
            }
            // srvs.at(req.getHost())->lctn.at(req.getReqURI())->show_all();
}