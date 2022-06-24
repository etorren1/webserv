#include "Server.hpp"

static void     ft(int) {
    // ignore SIGPIPE
}

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
    signal(SIGPIPE, ft);
}

void    Server::run( void ) {

    if (status & ~STOP)
        std::cout << GREEN << "Server running." << RESET << "\n";
    while(status & WORKING) {
        mainHandler();
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

void Server::clientRequest(const int socket) {
    if (client[socket]->status & IS_BODY) {

        writeLog(client[socket]->getServer()->get_access_log(), "" , client[socket]->getMessage());
        std::cout << YELLOW << "Client " << socket << " send BODY: " << RESET << "\n";
        std::cout << client[socket]->getMessage();

        std::cout << PURPLE << "end HEADER." << RESET << "\n";

        client[socket]->handleRequest(envp);
        client[socket]->parseLocation();
        client[socket]->initResponse(envp);
    } else {

        std::cout << YELLOW << "Client " << socket << " send HEADER: " << RESET << "\n";
        std::cout << client[socket]->getMessage();

        std::cout << PURPLE << "end HEADER." << RESET << "\n";

        client[socket]->handleRequest(envp);
        //client[socket]->status |= IS_BODY;
        Server_block * srv = getServerBlock( client[socket]->getHost() );
        if (srv == NULL) {
            std::cout << RED << "No such server with this host: has 400 exception \n" << RESET << "\n";
            throw codeException(400);
        }
        client[socket]->setServer(srv);
        writeLog(client[socket]->getServer()->get_access_log(), "Client " + itos(socket), client[socket]->getMessage());
        if (client[socket]->readComplete()) {
            client[socket]->parseLocation();
            //std::cout << "status after location - " << client[socket]->status << "\n";
            client[socket]->initResponse(envp);
        }
    }
}

void Server::mainHandler( void ) {
    int ret = poll(fds.data(), fds.size(), 0);
    if (ret != 0) {
        for (size_t id = 0; id < fds.size(); id++) {
            size_t socket = fds[id].fd;
            try {
                if (fds[id].revents & POLLIN) {
                    if (isServerSocket(socket))
                        connectClients(socket);
                    else if (readRequest(socket) <= 0)
                        disconnectClients(id);
                    else if (client[socket]->readComplete())
                        clientRequest(socket);
                }  
                else if (fds[id].revents & POLLOUT) {
                    if (!isServerSocket(socket) && client[socket]->status & REQ_DONE) {
                        // std::cout << YELLOW << "Send responce to " << socket << " socket" << RESET << "\n";
                        client[socket]->makeResponse(envp);
                    }
                }
            }
            catch(codeException& e) {
                client[socket]->handleError(e.getErrorCode());
            }
            fds[id].revents = 0;
        }
    }
}

int     Server::readRequest( const size_t socket ) {
    char buf[BUF_SIZE + 1];
    long bytesRead = 0;
    int rd;
    std::string text;

    if (client[socket]->getMessage().size() > 0) {
		text = client[socket]->getMessage();
        bytesRead = text.size();
    }
    if (client[socket]->status & IS_BODY)
        std::cout << RED << "Try read BODY\n";
    while ((rd = recv(socket, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        text += buf;
        if (client[socket]->status & IS_BODY)
            checkBodySize(socket, text);
        else if (text.rfind("\r\n\r\n") != std::string::npos)
            break;
    }
    if (client[socket]->status & IS_BODY)
        std::cout << "Readed " << bytesRead << " bytes." << RESET << "\n";
    client[socket]->setMessage(text);
    client[socket]->checkMessageEnd();
    //std::cout << CYAN << "bytes = " << bytesRead << " End read" << RESET << "\n";
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

Server::Server( char **envp, std::string nw_cfg_path ) {

    status = WORKING;
    this->envp = envp;
    nw_cfg_path.size() ? cfg_path = nw_cfg_path : cfg_path =  DEFAULT_PATH;
}

Server::~Server() {
    std::cout << "Destroyed.\n";
}
