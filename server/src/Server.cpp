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
    for (size_t i = 0; i < brokenhosts.size(); i++)
        srvs.erase(brokenhosts[i]);
    if (!srvs.size())
        closeServer(STOP);
    signal(SIGPIPE, NULL); // ignore sigpipe
}

void    Server::run( void ) {

    if (status & ~STOP)
        debug_msg(0, GREEN, "\e[1mServer running.");
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
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " bind failed");
    if (listen(newSrvSock, 10000) < 0)
        return closeVirtualServer(srv, newSrvSock, strerror(errno), "Host: " + hostname + ":" + port + " listen failed");
    fcntl(newSrvSock, F_SETFL, O_NONBLOCK);
    fds.push_back(newPoll);
    srvSockets.insert(newSrvSock);
    debug_msg(1, "Host: ", hostname, ":", port, " up succsesfuly");
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
            debug_msg(0, RED, "\e[1mShutdown server");
            closeServer(STOP);
        }
        else if (text == "RESTART") {
            debug_msg(0, YELLOW, "\e[1mRestarting server ... ");
            closeServer(RESTART);
            int fd = open(cfg_path.c_str(), O_RDONLY);
            config(fd);
            create();
        }
        else if (text == "HELP") {
            debug_msg(1, YELLOW, "Allowed command: ");
            debug_msg(1, " * STOP - shutdown server.", "\n");
            debug_msg(1, " * RESTART - restarting server.", "\n");
        }
        else
            debug_msg(1, RED, "Uncnown command. Use ", RESET, "HELP", RED, " for more information.");
    }
}

void Server::disconnectClients( const size_t id ) {
    debug_msg(1, "Client ", itos(fds[id].fd), " disconnected.");
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
        debug_msg(1,"New client on ", itos(newClientSock), " socket.");
    }
}

void Server::clientRequest(const int socket) {
    if (client[socket]->status & IS_BODY) {
        client[socket]->handleRequest();
        checkBodySize(socket, client[socket]->getStreamSize());
        client[socket]->parseLocation();
        client[socket]->initResponse(envp);
    } else {
        debug_msg(2, YELLOW, "Client ", itos(socket), " send HEADER: ", CYAN, "\n", client[socket]->getHeader());
        client[socket]->handleRequest();
        Server_block * srv = getServerBlock( client[socket]->getHost() );
        if (srv == NULL) {
            debug_msg(1, RED,  "No such server with this host: has 400 exception");
            throw codeException(400);
        }
        client[socket]->setServer(srv);
        writeLog(client[socket]->getServer()->get_access_log(), "Client " + itos(socket) + " header:", client[socket]->getHeader());
        if (client[socket]->readComplete()) {
            checkBodySize(socket, client[socket]->getStreamSize());
            client[socket]->parseLocation();
            client[socket]->initResponse(envp);
        }
    }
}

void Server::mainHandler( void ) {
    int ret = poll(fds.data(), fds.size(), 0);
    if (ret != 0) {
        for (size_t id = 0; id < fds.size(); id++) {
            size_t socket = fds[id].fd;
            bool   is_srv = isServerSocket(socket);
            try {
                if (fds[id].revents & POLLIN) {
                    if (is_srv)
                        connectClients(socket);
                    else if (readRequest(socket) <= 0) {
                        disconnectClients(id);
                        continue;
                    }
                    else if (client[socket]->readComplete())
                        clientRequest(socket);
                }  
                else if (fds[id].revents & POLLOUT) {
                    if (!is_srv && client[socket]->status & REQ_DONE) {
                        client[socket]->makeResponse();
                    }
                }
                if (!is_srv && checkTimeout(socket)) {
                    disconnectClients(id);
                    continue;;
                }
            }
            catch(codeException& e) {
                client[socket]->handleError(e.getErrorCode());
            }
            fds[id].revents = 0;
        }
    }
}

int     Server::readRequest( const size_t socket ) { // v2
    char buf[BUF_SIZE + 1];
    size_t bytesRead = 0;
    int rd, count = 0;

    bytesRead = client[socket]->getStreamSize();
    if ((rd = recv(socket, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        count += rd;
        client[socket]->getStream() << buf;
    }
    client[socket]->setStreamSize(bytesRead);
    client[socket]->checkMessageEnd();
    return (rd);
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
        (*it).second->cleaner();
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
    debug_msg(3, "Destroyed.");
}
