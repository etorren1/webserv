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
        // std::cout << GREEN << (*it).first << RESET << "\n";
        // (*it).second->show_all();
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
    size_t            newSrvSock;

    pe = getprotobyname("tcp");
    if ((newSrvSock = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == 0)
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " socket failed");
    int enable = 1;
    if (setsockopt(newSrvSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " setsockopt failed");
    newPoll.fd = newSrvSock;
    newPoll.events = POLLIN;
    newPoll.revents = 0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(hostname.c_str());
    address.sin_port = htons(atoi(port.c_str()));
    if (bind(newSrvSock, (struct sockaddr*)&address, sizeof(address)) < 0)
        // return closeVirtualServer(srv, newSrvSock, "error: bind failed: address already in use", "Host: " + hostname + ":" + port);
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " bind failed");
    if (listen(newSrvSock, 5) < 0)
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " listen failed");
    fcntl(newSrvSock, F_SETFL, O_NONBLOCK);
    /* 
    / F_SETFL устанавливет флаг O_NONBLOCK для подаваемого дескриптора 
    / O_NONBLOCK устанавливает  режим  неблокирования, 
    / что позволяет при ошибке вернуть чтение другим запросам 
    */
    fds.push_back(newPoll);
    srvSockets.insert(newSrvSock);
    std::cout << "Host: " << hostname << ":" << port << " up succsesfuly\n";
    return (1);
}

int    Server::closeVirtualServer( Server_block * srv, int sock, const std::string & error, const std::string & text ) {
    if (srv->get_error_log() != "off") {
        writeLog(srv->get_error_log(), error, text);
        std::cerr << "error: can not up domain: see error_log for more information\n";
    }
    else
        std::cerr << RED << "Host: " << srv->get_listen() << " break down" << RESET << "\n";
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
    int bytesRead = 0, rd;
    std::string text;
    while ((rd = read(fileno(stdin), buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
        text += buf;
        bytesRead += rd;
        if (text.find("\n") != std::string::npos) {
            text.erase(text.find("\n"), 1); break;
        }
    }
    if (bytesRead > 0) {
        if (text == "STOP") {
            std::cout << YELLOW << "Shutdown server\n" << RESET;
            closeServer(STOP);
        }
        else if (text == "RESTART") {
            std::cout << YELLOW << "Restarting server ... " << RESET;
            closeServer(RESTART);
            int fd = open(cfg_path.c_str(), O_RDONLY);
            config(fd);
            create();
        }
        else if (text == "HELP") {
            std::cout << YELLOW << "Allowed command: " << RESET << "\n\n";
            std::cout << " * STOP - shutdown server." << "\n\n";
            std::cout << " * RESTART - restarting server." << "\n\n";
        }
        else {
            std::cout << RED << "Uncnown command. Use " << RESET <<\
             "HELP" << RED << " for more information." << RESET << "\n";
        }
    }
}

void Server::disconnectClients( const size_t id ) {
    std::cout << "Client " << fds[id].fd << " disconnected." << "\n";
    close(fds[id].fd);

    delete client[fds[id].fd];
    client.erase(fds[id].fd);
    fds.erase(fds.begin() + id);
}

void Server::connectClients( const int & fd ) {
    int newClientSock;
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    if ((newClientSock = accept(fd, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen)) > 0) {
        struct pollfd nw;

        nw.fd = newClientSock;
        nw.events = POLLIN | POLLOUT;
        nw.revents = 0;
        fds.push_back(nw);   
        client.insert(std::make_pair(newClientSock, new Client(newClientSock)));
        fcntl(newClientSock, F_SETFL, O_NONBLOCK);
        std::cout << "New client on " << newClientSock << " socket." << "\n";
    }
}

void Server::clientRequest( void ) {
    int ret = poll(fds.data(), fds.size(), 0);
    if (ret != 0)    {
        for (size_t id = 0; id < fds.size(); id++) {
            size_t socket = fds[id].fd;
            try {
                if (fds[id].revents & POLLIN) {
                    if (isServerSocket(socket))
                        connectClients(socket);
                    else if (readRequest(socket) <= 0)
                        disconnectClients(id);
                    else if (!client[socket]->getBreakconnect())
                    {
                        // std::cout << YELLOW << "Client " << socket << " send: " << RESET << "\n";
                        // std::cout << client[socket]->getMessage();
                            client[socket]->handleRequest();
                    }
                }  else if (fds[id].revents & POLLOUT) {
                    // std::cout << "POLLOUT" << "\n";
                    if (!isServerSocket(socket) && client[socket]->status & REQ_DONE)
                        client[socket]->makeResponse();
                }
            }
            catch(codeException& e) {
                client[socket]->generateErrorPage(e.getErrorCode());;
                std::cerr << e.what() << '\n';
            }
            fds[id].revents = 0;
        }
    }
}

static bool checkConnection( const std::string & mess ) {
    if (mess.find_last_of("\n") != mess.size() - 1)
        return true;
    return false;
}

int     Server::readHeader( const size_t socket, std::string & text ) {
    char buf[BUF_SIZE + 1];
    int rd;
    int bytesRead = 0;

    if ((rd = recv(socket, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        text += buf;
        size_t pos = text.find("Host: ");
        if (pos != std::string::npos) {
            pos += 6;
            std::string host = text.substr(pos, text.find("\r\n", pos) - pos);
            Server_block * srv = getServerBlock(host);
            if (srv == NULL) {
                client[socket]->generateErrorPage(400);
                return (0);
            }
            client[socket]->setServer(srv);
        }
        else {
            client[socket]->generateErrorPage(400);
            return (0);
        }
    }
    return (bytesRead);
}

int     Server::readRequest( const size_t socket ) {
    char buf[BUF_SIZE + 1];
    long bytesRead = 0, bodyRead = 0;
    int rd;
    std::string text;

    if (client[socket]->getMessage().size() > 0)
		text = client[socket]->getMessage();
    else
        if ((bytesRead = readHeader(socket, text)) <= 0)
            return (0);
    if (checkBodySize(socket, text))
        return (0);
    while ((rd = recv(socket, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        text += buf;
        if (checkBodySize(socket, text))
            return (0);
    }
    while (text.find("\r") != std::string::npos)      // Удаляем символ возврата карретки
        text.erase(text.find("\r"), 1);               // из комбинации CRLF
    client[socket]->checkConnection(text);
    client[socket]->setMessage(text);
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
    for (client_iterator it = client.begin(); it != client.end(); it++) {
        delete (*it).second;
    }
    srvs.clear();
    client.clear();
    this->status = new_status;
}

void	Server::errorShutdown( int code, const std::string & path, const std::string & error, const std::string & text ) {
    if (path != "off") {
        writeLog(path, error, text);
        std::cerr << "error: see error_log for more information\n";
    }
    closeServer(STOP);
    exit(code);
}

Server::Server( std::string nw_cfg_path ) {

    status = WORKING;
    nw_cfg_path.size() ? cfg_path = nw_cfg_path : cfg_path =  DEFAULT_PATH;
}

Server::~Server() {
    std::cout << "Destroyed.\n";
}