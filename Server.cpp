#include "Server.hpp"

void    Server::create() {
    std::vector<std::string> brokenhosts;
    for ( srvs_iterator it = srvs.begin(); it != srvs.end(); it++) {
        std::string hostname;
        std::string port;
        getHostAndPort((*it).first, hostname, port);
        if (createVirtualServer(hostname, port, (*it).second) == -1) {
            brokenhosts.push_back((*it).first);
            delete (*it).second;
        }
    }
    for (int i = 0; i < brokenhosts.size(); i++)
        srvs.erase(brokenhosts[i]);
    if (!srvs.size())
        closeServer(STOP);
}

void    Server::run( void ) {
    if (status & ~STOP)
        std::cout << GREEN << "Server running." << RESET << "\n";
    while(status & WORKING) {
        clientRequest();
        consoleCommands();
    }
    if (status & RESTART) {
        status = WORKING;
        run();
    }
}

void    Server::getHostAndPort( const std::string & listen, std::string & hostname, std::string & port ) {
    size_t sep = listen.find(":");
    hostname = listen.substr(0, sep);
    port = listen.substr(sep + 1, listen.size() - sep - 1);
}

int    Server::createVirtualServer( const std::string & hostname, const std::string & port, Server_block * srv ) {
    struct sockaddr_in	address;
    struct protoent	*pe;
    struct pollfd   newPoll;
    int             newSrvSock;

    pe = getprotobyname("tcp");
    if ((newSrvSock = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == 0)
        return closeVirtualServer(srv, newSrvSock, "error: create socket failed.", "Host: " + hostname + ":" + port);
    int enable = 1;
    if (setsockopt(newSrvSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return closeVirtualServer(srv, newSrvSock, "error: setsockopt(SO_REUSEADDR) failed.", "Host: " + hostname + ":" + port);
    newPoll.fd = newSrvSock;
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(hostname.c_str());
    address.sin_port = htons(atoi(port.c_str()));
    if (bind(newSrvSock, (struct sockaddr*)&address, sizeof(address)) < 0)
        return closeVirtualServer(srv, newSrvSock, "error: bind failed: address already in use", "Host: " + hostname + ":" + port);
    if (listen(newSrvSock, 5) < 0)
        return closeVirtualServer(srv, newSrvSock, "error: listen failed.", "Host: " + hostname + ":" + port);
    fcntl(newSrvSock, F_SETFL, O_NONBLOCK);
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    /* 
    / F_SETFL устанавливет флаг O_NONBLOCK для подаваемого дескриптора 
    / O_NONBLOCK устанавливает  режим  неблокирования, 
    / что позволяет при ошибке вернуть чтение другим запросам 
    */
    fds.push_back(newPoll);
    mess.push_back("");
    cnct.push_back(false);
    srvSockets.insert(newSrvSock);
    std::cout << "Host: " << hostname << ":" << port << " up succsesfuly\n";
    return (1);
}

int    Server::closeVirtualServer( Server_block * srv, int sock, const std::string & error, const std::string & text ) {
    if (srv->get_error_log() != "off") {
        writeLog(srv->get_error_log(), error, text);
        std::cerr << "error: can not up domain: see error_log for more information\n";
    }
    for (lctn_iterator it = srv->lctn.begin(); it != srv->lctn.end(); it++) {
        delete (*it).second;
    }
    // delete srv;
    if (sock)
        close(sock);
    return (-1);
}

void Server::consoleCommands( void ) {
    char buf[BUF_SIZE + 1];
    int bytesRead = 0;
    int rd;
    std::string text;
    while ((rd = read(fileno(stdin), buf, BUF_SIZE)) > 0)
    {
        buf[rd] = 0;
        text += buf;
        bytesRead += rd;
        // std::cout << RED << "console command: " << RESET << text;
        if (text.find("\n") != std::string::npos) {
            text.erase(text.find("\n"), 1); 
            break;
        }
    }
    if (bytesRead > 0)
    {
        if (text == "STOP")
        {
            std::cout << YELLOW << "Shutdown server\n" << RESET;
            closeServer(STOP);
        }
        else if (text == "RESTART")
        {
            std::cout << YELLOW << "Restarting server ... " << RESET;
            closeServer(RESTART);
            int fd = open(cfg_path.c_str(), O_RDONLY);
            config(fd);
            // for ( srvs_iterator it = srvs.begin(); it != srvs.end(); it++) {
            //     std::cout << (*it).first << "\n";
            //     std::cout << (*it).second->get_access_log() << "\n";
            // }
            create();
        }
        else if (text == "HELP")
        {
            std::cout << YELLOW << "Allowed command: " << RESET << "\n\n";
            std::cout << " * STOP - shutdown server." << "\n\n";
            std::cout << " * RESTART - restarting server." << "\n\n";
        }
        else
        {
            std::cout << RED << "Uncnown command. Use " << RESET <<\
             "HELP" << RED << " for more information." << RESET << "\n";
        }
    }
}

void Server::disconnectClients( const size_t id ) {
    std::cout << "Client " << fds[id].fd << " disconnected." << "\n";
    close(fds[id].fd);

    fds.erase(fds.begin() + id);
    mess.erase(mess.begin() + id);
    cnct.erase(cnct.begin() + id);
}

void Server::connectClients( const int & fd ) {
    int newClientSock;
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    if ((newClientSock = accept(fd, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen)) > 0) {
        struct pollfd nw;

        nw.fd = newClientSock;
        nw.events = POLLIN;
        nw.revents = 0;
        fds.push_back(nw);
        mess.push_back("");
        cnct.push_back(false);
        std::cout << "New client on " << newClientSock << " socket." << "\n";
    }
}

void Server::clientRequest( void ) {
    int ret = poll(fds.data(), fds.size(), 0);
    if (ret != 0)    {
        for (size_t id = 0; id < fds.size(); id++) {
            if (fds[id].revents & POLLIN) {
                if (isServerSocket(fds[id].fd))
                    connectClients(fds[id].fd);
                else if (readRequest(id) <= 0)
                    disconnectClients(id);
                else if (!cnct[id]) {
                    // REQUEST PART

                    req.parseText(mess[id]);
                    parseLocation();
                    //

                    //  RESPONSE PART
                    // if (mess[id].size())
                    //     std::cout << YELLOW << "Client " << fds[id].fd << " send (full message): " << RESET << mess[id];
                        
					make_response(req, id);
                    //
                    mess[id] = "";
                }
                fds[id].revents = 0;
            }
        }
    }
}

static bool checkConnection( const std::string & mess ) {
    if (mess.find_last_of("\n") != mess.size() - 1)
        return true;
    return false;
}

int  Server::readRequest( const size_t id ) {
    char buf[BUF_SIZE + 1];
    int bytesRead = 0;
    int rd;
    std::string text;
    if (mess[id].size() > 0)
		text = mess[id];
    while ((rd = recv(fds[id].fd, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        text += buf;
        if (text.find("\n") != std::string::npos)
            break;
    }
    while (text.find("\r") != std::string::npos)      // Удаляем символ возврата карретки
        text.erase(text.find("\r"), 1);               // из комбинации CRLF
    if (text.size() > BUF_SIZE) {
        text.replace(BUF_SIZE - 2, 2, "\r\n");
        std::cout << RED << "ALERT! text more than " << BUF_SIZE << " bytes!" << RESET << "\n";
    }
    cnct[id] = checkConnection(text);
    mess[id] = text;
    return (bytesRead);
}

void    Server::closeServer( int new_status ) {

    size_t count = fds.size();
    for (size_t i = 0; i < count; i++)
        close(fds[i].fd);
    fds.clear();
    if (http != NULL)
        delete http;
    for (srvs_iterator it = srvs.begin(); it != srvs.end(); it++) {
        for (lctn_iterator jt = (*it).second->lctn.begin(); jt != (*it).second->lctn.end(); jt++) {
            delete (*jt).second;
        }
        delete (*it).second;
    }
    srvs.clear();
    this->status = new_status;
}

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

void	Server::errorShutdown( int code, const std::string & path, const std::string & error, const std::string & text ) {
    if (path != "off") {
        writeLog(path, error, text);
        std::cerr << "error: see error_log for more information\n";
    }
    closeServer(STOP);
    exit(code);
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
    try    {
        srvs.at(req.getHost())->lctn.at(req.getReqURI())->show_all();
    }
    catch(const std::exception& e)    {
        try        {
            size_t pos = req.getHost().find(":");
            if (pos != std::string::npos) {
                std::string ip = "0.0.0.0" + req.getHost().substr(pos);
                req.setHost(ip);
            }
            // std::cout << "req.getHost() = "<< req.getHost() << "\n";
            std::cout << "req.getReqURI() = "<< req.getReqURI() << "\n";
            std::vector<std::string> vec = req.getDirs();
            std::string tmp = "";
            location = "";
            for (size_t i = 0; i < vec.size(); i++) {
                try {
                    std::cout << "location which need found = " << srvs.at(req.getHost())->lctn.at(vec[i]) << "\n";
                    tmp = srvs.at(req.getHost())->lctn.at(vec[i])->get_root();
                    std::cout << "vec[" << i << "] = " << vec[i] << "\n";
                    std::cout << "tmp = " << tmp << "\n";
                    if (tmp.length() > location.length())
                        location = tmp;
                    std::cout << "finded location = " << location << "\n";
                }
                catch(std::exception &e) { std::cout << "\n"; }
            }
                std::cout << "location without root = " << location << "\n";
                location += vec[0];
                std::cout << "location with root = " << location << "\n";
                if (existDir(location.c_str())) {
                    int ret = open(location.c_str(), O_RDONLY);
                    // struct stat s;
                    if (!access(location.c_str(), 4)) {
                        if (req.getMIMEType() == "none") {
                            std::cout << "Directory exist\n";
                        } else {
                            FILE *file;
                            file = fopen("index.html", "r");
                            if (file != NULL) {
                                //make_response ???
                            } else {
                                throw(codeException(404));
                            }
                        }
                    }
                    else {
                        std::cout << "Permission denied\n";
                        throw(codeException(403));
                    }
                } else {
                    std::cout << "Dir " << location << "don't exist\n";
                    throw(codeException(400));
                }
            // srvs.at(req.getHost())->lctn.at(req.getReqURI())->show_all();
        }
        catch(const codeException& e)        {
            throw(codeException(400));
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    }
}

Server::Server( std::string nw_cfg_path ) {

    status = WORKING;
    //Для POST браузер сначала отправляет заголовок, сервер отвечает 100 continue, браузер 
    // отправляет данные, а сервер отвечает 200 ok (возвращаемые данные).
    nw_cfg_path.size() ? cfg_path = nw_cfg_path : cfg_path =  DEFAULT_PATH;
    this->resCode.insert(std::make_pair(100, "Continue"));
    this->resCode.insert(std::make_pair(101, "Switching Protocols"));
    this->resCode.insert(std::make_pair(200, "OK"));
    this->resCode.insert(std::make_pair(201, "Created"));
    this->resCode.insert(std::make_pair(202, "Accepted"));
    this->resCode.insert(std::make_pair(203, "Non-Authoritative Information"));
    this->resCode.insert(std::make_pair(204, "No Content"));
    this->resCode.insert(std::make_pair(304, "Not Modified"));
    this->resCode.insert(std::make_pair(400, "Bad Request"));
    this->resCode.insert(std::make_pair(401, "Unauthorized"));
    this->resCode.insert(std::make_pair(402, "Payment Required"));
    this->resCode.insert(std::make_pair(403, "Forbidden"));
    this->resCode.insert(std::make_pair(404, "Not Found"));
    this->resCode.insert(std::make_pair(405, "Method Not Allowed"));
    this->resCode.insert(std::make_pair(406, "Not Acceptable"));
    this->resCode.insert(std::make_pair(407, "Proxy Authentication Required"));
    this->resCode.insert(std::make_pair(408, "Request Timeout"));
    this->resCode.insert(std::make_pair(409, "Conflict"));
    this->resCode.insert(std::make_pair(500, "Internal Server Error"));
    this->resCode.insert(std::make_pair(501, "Not Implemented"));
    this->resCode.insert(std::make_pair(502, "Bad Gateway"));
    this->resCode.insert(std::make_pair(503, "Service Unavailable"));
    this->resCode.insert(std::make_pair(504, "Gateway Timeout"));
}

Server::~Server() {
    std::cout << "Destroyed.\n";
}
