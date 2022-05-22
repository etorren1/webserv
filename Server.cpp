#include "Server.hpp"

void Server::create( void ) {
    struct protoent	*pe;

    pe = getprotobyname("tcp");
    if ((srvFd = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int enable = 1;
    if (setsockopt(srvFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    srvPoll.fd = srvFd;
    srvPoll.events = POLLIN;
    srvPoll.revents = 0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    // address.sin_addr.s_addr = inet_addr("ip_addr");
    address.sin_port = htons(atoi(conf.port.c_str()));
    if (bind(srvFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(srvFd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    fcntl(srvFd, F_SETFL, O_NONBLOCK);
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    /* 
    / F_SETFL устанавливет флаг O_NONBLOCK для подаваемого дескриптора 
    / O_NONBLOCK устанавливает  режим  неблокирования, 
    / что позволяет при ошибке вернуть чтение другим запросам 
    */
}

void Server::run( void ) {
    std::cout << GREEN << "Server running." << RESET << "\n";
    while(status & WORKING) {
        connectUsers();
        clientRequest();
        consoleCommands();
    }
    if (status & RESTART) {
        status = WORKING;
        run();
    }
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

void Server::connectUsers( void ) {
    int new_client_fd;
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    int ret = poll(&srvPoll, 1, 0);
    if (ret != 0) {
        if (srvPoll.revents & POLLIN) {
            if ((new_client_fd = accept(srvFd, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen)) > 0) {
                struct pollfd nw;

                nw.fd = new_client_fd;
                nw.events = POLLIN;
                nw.revents = 0;
                fds.push_back(nw);
                std::cout << "New client on " << new_client_fd << " socket." << "\n";
            }
            srvPoll.revents = 0;
        }
    }
}

void Server::clientRequest( void ) {
    int ret = poll(fds.data(), fds.size(), 0);
    if (ret != 0)    {
        for (size_t id = 0; id < fds.size(); id++) {
            if (fds[id].revents & POLLIN) {
                if (readRequest(id) <= 0)
                {
                    std::cout << "Client " << fds[id].fd << " disconnected." << "\n";
                    close(fds[id].fd);
                }
                // else if (!userData[id]->getBreakconnect())
                    // request.parseText(text);
                //     executeCommand(id);
                fds[id].revents = 0;
            }
        }
    }
}

int  Server::readRequest( const size_t id ) {
    char buf[BUF_SIZE + 1];
    int bytesRead = 0;
    int rd;
    std::string text;
    // if (userData[id]->messages.size() > 0)
	// 	text = userData[id]->messages.front();
    while ((rd = recv(fds[id].fd, buf, BUF_SIZE, 0)) > 0) {
        buf[rd] = 0;
        bytesRead += rd;
        text += buf;
        if (text.find("\n") != std::string::npos)
            break;
    }
    while (text.find("\r") != std::string::npos)      // Удаляем символ возврата карретки
        text.erase(text.find("\r"), 1);               // из комбинации CRLF
    if (text.size() > 2048)   //Длина запроса Не более 2048 символов
    {
        text.replace(2046, 2, "\r\n");
        std::cout << RED << "ALERT! text more than 2048 bytes!" << RESET << "\n";
    }
    // userData[id]->checkConnection(text);
    // userData[id]->messages = split(text, "\n");
    if (text.size())
        std::cout << YELLOW << "Client " << fds[id].fd << " send: " << RESET << text;

    if (text.find("localhost:8080") != std::string::npos) // fix close server then client send message
    {

        std::stringstream response_body;
        std::stringstream response;
        int fd;
        size_t result;
        response_body << "<title>Test C++ HTTP Server</title>\n"
            << "<h1>Test page</h1>\n"
            << "<p>This is body of the test page...</p>\n"
            << "<h2>Request headers</h2>\n"
            << "<pre>" << buf << "</pre>\n"
            << "<em><small>Test C++ Http Server</small></em>\n";

        // Формируем весь ответ вместе с заголовками
        response << "HTTP/1.1 200 OK\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << response_body.str().length()
            << "\r\n\r\n"
            << response_body.str();

        // Отправляем ответ клиенту с помощью функции send
        result = send(fds[id].fd, response.str().c_str(),
            response.str().length(), 0);
        
    }
    req.parseText(text);
    return (bytesRead);
}

void    Server::closeServer( int status ) {
    close(srvFd);
    if (conf.error_fd > 2)
        close(conf.error_fd);
    if (conf.access_fd > 2)
        close(conf.access_fd);
    size_t count = fds.size();
    for (size_t i = 0; i < count; i++)
        close(fds[i].fd);
    fds.clear();
    this->status = status;
}

void    Server::writeLog( int dest, const std::string & header, const std::string & text ) {
    int fd;
    if (this->flags & ERR_LOG && dest & ERR_LOG)
        fd = conf.error_fd;
    else if (this->flags & ACS_LOG && dest & ACS_LOG)
        fd = conf.access_fd;
    else
        fd = 0;
    if (fd) {
        std::time_t result = std::time(nullptr);
        std::string time = std::asctime(std::localtime(&result));
        time = "[" + time.erase(time.size() - 1) + "] ";
        write(fd, time.c_str(), time.size());
        write(fd, header.c_str(), header.size());
        write(fd, "\n\n", 2);
        write(fd, text.c_str(), text.size());
        write(fd, "\n\n", 2);
    }
}

void	Server::errorShutdown( int code, const std::string & error, const std::string & text ) {
    writeLog(ERR_LOG, error, text);
    if (this->flags & ERR_LOG)
        std::cerr << "error: see error.log for more information\n";
    closeServer(STOP);
    exit(code);
}

Server::Server( const int & config_fd ) {

    parseConfig(config_fd);

    status = WORKING;
    flags = 0;
}

Server::~Server() {
    std::cout << "Destroyed.\n";
}
