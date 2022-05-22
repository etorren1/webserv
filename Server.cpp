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
    address.sin_port = htons(srvPort);
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
            close(srvFd);
            size_t count = userFds.size();
            for (size_t i = 0; i < count; i++)
                close(userFds[i].fd);
            userFds.clear();
            status = 0;
        }
        else if (text == "RESTART")
        {
            std::cout << YELLOW << "Restarting server ... " << RESET;
            close(srvFd);
            size_t count = userFds.size();
            for (size_t i = 0; i < count; i++)
                close(userFds[i].fd);
            userFds.clear();
            create();
            status = RESTART;
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
                userFds.push_back(nw);
                std::cout << "New client on " << new_client_fd << " socket." << "\n";
            }
            srvPoll.revents = 0;
        }
    }
}

void Server::clientRequest( void ) {
    int ret = poll(userFds.data(), userFds.size(), 0);
    if (ret != 0)    {
        for (size_t id = 0; id < userFds.size(); id++) {
            if (userFds[id].revents & POLLIN) {
                if (readRequest(id) <= 0)
                {
                    std::cout << "Client " << userFds[id].fd << " disconnected." << "\n";
                    close(userFds[id].fd);
                }
                // else if (!userData[id]->getBreakconnect())
                    // request.parseText(text);
                //     executeCommand(id);
                userFds[id].revents = 0;
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
    while ((rd = recv(userFds[id].fd, buf, BUF_SIZE, 0)) > 0) {
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
        std::cout << YELLOW << "User " << userFds[id].fd << " send: " << RESET << text;

    if (text.find("localhost:8080") != std::string::npos)
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
        result = send(userFds[id].fd, response.str().c_str(),
            response.str().length(), 0);
        
    }
    // req.parseText(text);
    return (bytesRead);
}

Server::Server( const std::string & config ) {
    // try  {
    //     if (_port.find_first_not_of("0123456789") != std::string::npos)
    //         throw std::invalid_argument("Port must contain only numbers");
    //     srvPort = atoi(_port.c_str());
    //     if (srvPort < 1000 || srvPort > 65555) // надо взять правильный рендж портов...
    //         throw std::invalid_argument("Port out of range");
    // }
    // catch ( std::exception & e) {
    //     std::cerr << e.what() << "\n";
    //     exit(EXIT_FAILURE);
    // }
    status = WORKING;

    parseConfig(config);

    std::cout << "Done!\n";
}

Server::~Server() {
    std::cout << "Destroyed.\n";
}
